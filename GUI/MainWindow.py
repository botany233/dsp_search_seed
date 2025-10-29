import signal
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

from .Compoents import UserLayout
from .Compoents.Widgets.line_edit import LimitLineEdit
from .Widgets import SortTreeWidget, WaitRing

from .seed_viewer.MainInterface import ViewerInterface

import math

class MainWindow(FluentWindow):
    def __init__(self):
        super().__init__()
        self.setObjectName("MainWindow")

        setThemeColor("#409EFF")

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
        self.setWindowTitle("戴森球计划种子搜索器 made by 前前&哒哒")
        self.setWindowIcon(QIcon(r".\assets\icon.png"))
        width: int = 1260
        height: int = width // 16 * 8
        self.resize(width, height)

        qss = """StackedWidget{background-color: white;}"""
        setCustomStyleSheet(self.stackedWidget, qss, qss)
        setTheme(Theme.LIGHT)
        self.setMicaEffectEnabled(True)
        self.titleBar.raise_()

        self.navigationInterface.setExpandWidth(233)
        self.navigationInterface.displayModeChanged.connect(self._on_display_mode_changed)

        
        self.searchInterface = QFrame(self)
        self.searchInterface.setObjectName("searchLayout")
        self.searchLayout = VBoxLayout(self.searchInterface)
        self.viewerInterface = ViewerInterface(self)
        self.viewerInterface.setObjectName("viewerLayout")

        self.addSubInterface(self.searchInterface, icon=FluentIcon.SEARCH_MIRROR, text="种子搜索器")
        self.addSubInterface(self.viewerInterface, icon=FluentIcon.VIEW, text="种子查看器")
        
        self.__build__()

        screen = QApplication.primaryScreen()
        screen_geometry = screen.geometry()
        x = (screen_geometry.width() - width) // 2
        y = (screen_geometry.height() - height) // 2
        self.move(x, y)

        signal.signal(signal.SIGINT, self.__handle_exit__)
        self.search_thread = SearchThread(self)
        SearchMessages.searchEnd.connect(self._on_search_finish)

        self.waiting_ring = WaitRing(self.window())
        self.waiting_ring.stop()

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
        return super().closeEvent(e)

    def __init__layout__(self):
        self.topLayout = QGridLayout()
        self.searchLayout.addLayout(self.topLayout)
        self.middleLayout = QHBoxLayout()
        self.searchLayout.addLayout(self.middleLayout, stretch=1)
        self.userLayout = UserLayout()
        self.searchLayout.addLayout(self.userLayout)
        self.buttonLayout = QHBoxLayout()
        self.searchLayout.addStretch()
        self.searchLayout.addLayout(self.buttonLayout)
        self.searchLayout.setContentsMargins(10, 10, 10, 10)

    def __init__widgets__(self):
        self.label_seed_range = BodyLabel("种子范围:")
        self.input_seed_start = LimitLineEdit("start_seed", min_value=0, max_value=99999999, default_value=0, empty_invisible=False)
        # self.input_seed_start.setMinimumWidth(80)
        self.input_seed_end = LimitLineEdit("end_seed", min_value=0, max_value=99999999, default_value=99999, empty_invisible=False)
        # self.input_seed_end.setMinimumWidth(80)
        self.label_star_num = BodyLabel("恒星数:")
        self.input_star_num_start = LimitLineEdit("start_star_num", min_value=32, max_value=64, default_value=32, empty_invisible=False)
        self.input_star_num_end = LimitLineEdit("end_star_num", min_value=32, max_value=64, default_value=64, empty_invisible=False)
        self.label_batch_size = BodyLabel("批处理大小:")
        self.input_batch_size = LimitLineEdit("batch_size", min_value=1, max_value=4096, default_value=64, empty_invisible=False)
        self.label_thread_num = BodyLabel("进程数:")
        self.input_thread_num = LimitLineEdit("max_thread", min_value=1, max_value=128, default_value=cpu_count(), empty_invisible=False)
        self.button_start = PushButton("开始搜索")
        self.button_start.clicked.connect(self.__on_button_start_clicked)

        self.button_stop = PushButton("停止搜索")
        self.button_stop.clicked.connect(self.__on_button_stop_clicked)
        self.button_stop.setEnabled(False)

        self.tree_view = SortTreeWidget()

    def __build__(self):
        self.__init__layout__()
        self.__init__widgets__()

        self.topLayout.addWidget(self.label_seed_range, 0, 0)
        self.topLayout.addWidget(self.input_seed_start, 0, 1)
        self.topLayout.addWidget(BodyLabel("至"), 0, 2)
        self.topLayout.addWidget(self.input_seed_end, 0, 3)
        # self.topLayout.addWidget(self.seed_step_range, 0, 4)
        self.topLayout.addWidget(self.label_star_num, 0, 4)
        self.topLayout.addWidget(self.input_star_num_start, 0, 5)
        self.topLayout.addWidget(BodyLabel("至"), 0, 6)
        self.topLayout.addWidget(self.input_star_num_end, 0, 7)
        self.topLayout.addWidget(self.label_batch_size, 0, 8)
        self.topLayout.addWidget(self.input_batch_size, 0, 9)
        self.topLayout.addWidget(self.label_thread_num, 0, 10)
        self.topLayout.addWidget(self.input_thread_num, 0, 11)

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

    def _on_display_mode_changed(self, mode):
        if mode == NavigationDisplayMode.MENU:
            self.titleBar.titleLabel.setHidden(True)
        else:
            self.titleBar.titleLabel.setHidden(False)

    def _on_search_finish(self):
        self.button_start.setEnabled(True)
        self.button_stop.setEnabled(False)

    def __on_button_start_clicked(self):
        if self.search_thread.isRunning():
            log.info("搜索线程已在运行中，请勿重复点击开始按钮")
            return

        seeds = (cfg.config.start_seed, cfg.config.end_seed)
        batch_size = cfg.config.batch_size
        self.userLayout.seeds = seeds
        self.userLayout.batch_size = batch_size

        log.info("开始搜索")
        self.button_start.setEnabled(False)
        self.userLayout.progressBar.setMaximum(math.ceil((seeds[1]-seeds[0]+1)/batch_size))
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
