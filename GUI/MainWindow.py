import signal
from csv import reader
from multiprocessing import cpu_count
from PySide6.QtCore import Qt, QEvent
from PySide6.QtGui import QColor, QCursor, QFont, QFontDatabase, QIcon
from PySide6.QtWidgets import (
    QApplication,
    QGridLayout,
    QHBoxLayout,
    QSizePolicy,
    QToolTip,
    QTreeWidgetItem,
    QFrame,
    QFileDialog
)
from qfluentwidgets import (
    FluentWindow,
    BodyLabel,
    ComboBox,
    FlowLayout,
    Pivot,
    ProgressRing,
    PushButton,
    TextEdit,
    Theme,
    VBoxLayout,
    qconfig,
    setTheme,
    setThemeColor,
    MessageBox,
    FluentIcon,
    NavigationDisplayMode,
    setCustomStyleSheet
)

from config import cfg
from logger import log

from .search_seed import SearchThread
from .Messenger import SearchMessages

from .Compoents import UserLayout, ImportSeedInfo
from .Compoents.Widgets.line_edit import LimitLineEdit
from .Widgets import SortTreeWidget, WaitRing
from .Compoents.about_interface import AboutInterface

from .seed_viewer.MainInterface import ViewerInterface
from .tutorial import TutorialInterface
from .setting_interface import SettingInterface
from GUI.dsp_icons import AppIcons
from CApi import SeedManager

import math

class MainWindow(FluentWindow):
    def __init__(self):
        super().__init__()
        self.setObjectName("MainWindow")

        setThemeColor("#409EFF")
        setTheme(Theme.AUTO)
        # 设置字体，提供多个备选
        font_families = [
            "Microsoft YaHei",  # 微软雅黑
            "微软雅黑",  # 微软雅黑中文名
            "Segoe UI",  # Windows 现代UI字体
            "PingFang SC",  # macOS 中文字体
            "Hiragino Sans GB",  # macOS 备选中文字体
            "WenQuanYi Micro Hei",  # Linux 中文字体
            "Noto Sans CJK SC",  # 跨平台中文字体
            "sans-serif",  # 最后回退到无衬线字体
        ]

        qconfig.fontFamilies.value = font_families

        self.titleBar.titleLabel.setStyleSheet("""
            QLabel{
                background: transparent;
                font-family: "Microsoft YaHei", "微软雅黑", "PingFang SC", "Hiragino Sans GB", "Segoe UI", sans-serif;
                font-size: 13px;
                padding: 0 4px
            }
        """)
        self.setWindowTitle("戴森球计划种子搜索查看器 made by 前前&哒哒")
        self.setWindowIcon(AppIcons.LOGO.icon())
        width: int = 1260
        height: int = width // 16 * 8
        self.resize(width, height)

        lt_qss = """StackedWidget{background-color: #FCFCFC;}"""
        dk_qss = """StackedWidget{background-color: #181818;}"""
        setCustomStyleSheet(self.stackedWidget, lt_qss, dk_qss)

        self.setMicaEffectEnabled(True)
        self.titleBar.raise_()

        self.navigationInterface.setExpandWidth(233)
        self.navigationInterface.displayModeChanged.connect(self._on_display_mode_changed)

        
        self.searchInterface = QFrame(self)
        self.searchInterface.setObjectName("searchLayout")
        self.searchLayout = VBoxLayout(self.searchInterface)
        self.viewerInterface = ViewerInterface(self)
        self.viewerInterface.setObjectName("viewerLayout")
        self.tutorialInterface = TutorialInterface()
        self.tutorialInterface.setObjectName("tutorialLayout")
        self.settingInterface = SettingInterface()
        self.settingInterface.setObjectName("settingLayout")
        self.aboutInterface = AboutInterface(self)
        self.aboutInterface.setObjectName("aboutLayout")

        self.addSubInterface(self.searchInterface, icon=FluentIcon.SEARCH_MIRROR, text="种子搜索器")
        self.addSubInterface(self.viewerInterface, icon=FluentIcon.VIEW, text="种子查看器")
        self.addSubInterface(self.tutorialInterface, icon=FluentIcon.HELP, text="使用教程")
        self.addSubInterface(self.settingInterface, icon=FluentIcon.SETTING, text="设置")
        self.addSubInterface(self.aboutInterface, icon=FluentIcon.INFO, text="关于")

        self.seed_manager = SeedManager()
        self.__build__()

        screen = QApplication.primaryScreen()
        screen_geometry = screen.geometry()
        x = (screen_geometry.width() - width) // 2
        y = (screen_geometry.height() - height) // 2
        self.move(x, y)

        signal.signal(signal.SIGINT, self.__handle_exit__)
        self.search_thread = SearchThread(self.seed_manager, self)
        SearchMessages.searchEnd.connect(self._on_search_finish)

        self.waiting_ring = WaitRing(self.window())
        self.waiting_ring.stop()

        self.__switch_search_mode(cfg.config.search_mode)

    def _waiting_thread_finish(self, thread) -> bool:
        message_box = MessageBox(
            "有任务正在进行中，是否强制退出？",
            "强制退出可能会导致部分结果未保存，建议先停止任务。",
            self.window(),
        )
        if message_box.exec() != 1:
            return True  # 取消关闭事件
        self.waiting_ring.start()
        thread.terminate()
        while thread.isRunning():
            QApplication.processEvents()
        return False

    def closeEvent(self, e: QEvent):
        if self.search_thread.isRunning():
            if self._waiting_thread_finish(self.search_thread):
                e.ignore()
                return
        elif self.viewerInterface.sort_thread.isRunning():
            if self._waiting_thread_finish(self.viewerInterface.sort_thread):
                e.ignore()
                return
        self.viewerInterface.get_data_manager.shutdown()
        return super().closeEvent(e)

    def __init__layout__(self):
        self.topLayout = QHBoxLayout()
        self.searchLayout.addLayout(self.topLayout)
        self.middleLayout = QHBoxLayout()
        self.searchLayout.addLayout(self.middleLayout, stretch=1)
        self.userLayout = UserLayout()
        self.userLayout.search_mode_switch.checkedChanged.connect(self._on_search_mode_changed)
        self.searchLayout.addLayout(self.userLayout)
        self.buttonLayout = QHBoxLayout()
        self.searchLayout.addStretch()
        self.searchLayout.addLayout(self.buttonLayout)
        self.searchLayout.setContentsMargins(10, 10, 10, 10)

    def __init__widgets__(self):
        self.add_file_button = PushButton("导入种子")
        self.add_file_button.clicked.connect(self.__on_add_file_button_clicked)
        self.del_file_button = PushButton("清空种子")
        self.del_file_button.clicked.connect(self.__on_del_file_button_clicked)
        self.seed_info_label = ImportSeedInfo(self.seed_manager)
        self.label_seed_range = BodyLabel("种子范围:")
        self.input_seed_start = LimitLineEdit("start_seed", min_value=0, max_value=99999999, default_value=0, empty_invisible=False)
        # self.input_seed_start.setMinimumWidth(80)
        self.input_seed_end = LimitLineEdit("end_seed", min_value=0, max_value=99999999, default_value=99999, empty_invisible=False)
        # self.input_seed_end.setMinimumWidth(80)
        self.label_star_num = BodyLabel("恒星数:")
        self.input_star_num_start = LimitLineEdit("start_star_num", min_value=32, max_value=64, default_value=32, empty_invisible=False)
        self.input_star_num_end = LimitLineEdit("end_star_num", min_value=32, max_value=64, default_value=64, empty_invisible=False)
        # self.label_batch_size = BodyLabel("批处理大小:")
        # self.input_batch_size = LimitLineEdit("batch_size", min_value=1, max_value=4096, default_value=64, empty_invisible=False)
        # self.label_thread_num = BodyLabel("进程数:")
        # self.input_thread_num = LimitLineEdit("max_thread", min_value=1, max_value=128, default_value=cpu_count(), empty_invisible=False)
        self.button_start = PushButton("开始搜索")
        self.button_start.clicked.connect(self.__on_button_start_clicked)

        self.button_stop = PushButton("停止搜索")
        self.button_stop.clicked.connect(self.__on_button_stop_clicked)
        self.button_stop.setEnabled(False)

        self.tree_view = SortTreeWidget()

    def __build__(self):
        self.__init__layout__()
        self.__init__widgets__()

        self.topLayout.addWidget(self.add_file_button)
        self.topLayout.addWidget(self.seed_info_label)
        self.topLayout.addWidget(self.del_file_button)

        self.topLayout.addWidget(self.label_seed_range)
        self.topLayout.addWidget(self.input_seed_start)
        self.seed_to_label = BodyLabel("至")
        self.topLayout.addWidget(self.seed_to_label)
        self.topLayout.addWidget(self.input_seed_end)
        # self.topLayout.addWidget(self.seed_step_range, 0, 4)
        self.topLayout.addWidget(self.label_star_num)
        self.topLayout.addWidget(self.input_star_num_start)
        self.star_to_label = BodyLabel("至")
        self.topLayout.addWidget(self.star_to_label)
        self.topLayout.addWidget(self.input_star_num_end)
        # self.topLayout.addWidget(self.label_batch_size)
        # self.topLayout.addWidget(self.input_batch_size)
        # self.topLayout.addWidget(self.label_thread_num)
        # self.topLayout.addWidget(self.input_thread_num)

        self.middleLayout.addWidget(self.tree_view)

        self.buttonLayout.addWidget(self.button_start)
        self.buttonLayout.addWidget(self.button_stop)

        self.__recover_condition()

    def __recover_condition(self):
        galaxy_condition = cfg.config.galaxy_condition
        galaxy_leaf = self.tree_view.tree.addLeaf(galaxy_condition)
        for star_condition in galaxy_condition.star_condition:
            star_leaf = galaxy_leaf.addStarLeaf(star_condition)
            for planet_condition in star_condition.planet_condition:
                star_leaf.addPlanetLeaf(planet_condition)
        for planet_condition in galaxy_condition.planet_condition:
            galaxy_leaf.addPlanetLeaf(planet_condition)

    def __handle_exit__(self, signum, frame):
        import sys

        self.close()
        sys.exit(0)

    def _on_search_mode_changed(self, checked: bool):
        self.__switch_search_mode(0 if not checked else 1)

    def __switch_search_mode(self, mode: int):
        if mode == 0:
            self.label_seed_range.setHidden(False)
            self.input_seed_start.setHidden(False)
            self.input_seed_end.setHidden(False)
            self.label_star_num.setHidden(False)
            self.input_star_num_start.setHidden(False)
            self.input_star_num_end.setHidden(False)
            self.seed_to_label.setHidden(False)
            self.star_to_label.setHidden(False)

            self.add_file_button.setHidden(True)
            self.del_file_button.setHidden(True)
            self.seed_info_label.setHidden(True)
        else:
            self.label_seed_range.setHidden(True)
            self.input_seed_start.setHidden(True)
            self.input_seed_end.setHidden(True)
            self.label_star_num.setHidden(True)
            self.input_star_num_start.setHidden(True)
            self.input_star_num_end.setHidden(True)
            self.seed_to_label.setHidden(True)
            self.star_to_label.setHidden(True)

            self.add_file_button.setHidden(False)
            self.del_file_button.setHidden(False)
            self.seed_info_label.setHidden(False)

    def _on_display_mode_changed(self, mode):
        if mode == NavigationDisplayMode.MENU:
            self.titleBar.titleLabel.setHidden(True)
        else:
            self.titleBar.titleLabel.setHidden(False)

    def _on_search_finish(self):
        self.button_start.setEnabled(True)
        self.button_stop.setEnabled(False)

    def __on_add_file_button_clicked(self):
        if self.search_thread.isRunning():
            return
        file_paths, _ = QFileDialog.getOpenFileNames(
            self,
            '选择CSV文件',
            '',
            'CSV Files (*.csv);'
        )
        for file_path in file_paths:
            with open(file_path, "r", encoding="utf-8") as f:
                data = reader(f)
                for i, row in enumerate(data):
                    try:
                        seed = int(row[0])
                        star_num = int(row[1])
                    except Exception:
                        continue

                    if not (0 <= seed <= 99999999 and 32 <= star_num <= 64):
                        continue

                    self.seed_manager.add_seed(seed, star_num)

                    if i % 1000 == 0:
                        QApplication.processEvents()
        self.seed_info_label.fresh()

    def __on_del_file_button_clicked(self):
        if self.search_thread.isRunning():
            return
        self.seed_manager.clear()
        self.seed_info_label.fresh()

    def __on_button_start_clicked(self):
        if self.search_thread.isRunning():
            log.info("搜索线程已在运行中，请勿重复点击开始按钮")
            return
        if cfg.config.search_mode == 1 and self.seed_manager.get_seeds_count() == 0:
            log.info("当前无可搜索种子，请先导入种子文件或修改搜索范围")
            return

        seeds = (cfg.config.start_seed, cfg.config.end_seed)
        star_nums = (cfg.config.start_star_num, cfg.config.end_star_num)
        # self.userLayout.seeds = seeds

        log.info("开始搜索")
        self.button_start.setEnabled(False)
        self.userLayout.progressBar.setMaximum((seeds[1]-seeds[0]+1)*(star_nums[1]-star_nums[0]+1))
        self.userLayout.seedInfoLabel.setText("")
        log.debug(self.userLayout.progressBar.maximum())
        self.search_thread.start()
        self.button_stop.setEnabled(True)

    def __on_button_stop_clicked(self):
        if self.search_thread.isRunning():
            self.search_thread.terminate()
            log.info("搜索已停止")

if __name__ == "__main__":
    import sys

    from PySide6.QtWidgets import QApplication

    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
