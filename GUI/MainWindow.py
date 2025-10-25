import signal

from PySide6.QtCore import Qt, QEvent
from PySide6.QtGui import QColor, QCursor, QFont, QFontDatabase, QIcon
from PySide6.QtWidgets import (
    QApplication,
    QGridLayout,
    QHBoxLayout,
    QSizePolicy,
    QToolTip,
    QTreeWidgetItem,
)
from qfluentwidgets import (
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
)
from qfluentwidgets.components.widgets.frameless_window import FramelessWindow
from qframelesswindow import StandardTitleBar

from config import cfg
from logger import log
from GUI import liquid, planet_types, singularity, star_types, vein_names

from .search_seed import SearchThread
from .Messenger import SearchMessages

from .Compoents import LabelWithComboBox, UserLayout
from .Compoents.Widgets.line_edit import ConfigLineEdit, LabelWithLineEdit, LimitLineEdit
from .Widgets import SortTreeWidget

class MainWindow(FramelessWindow):
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

        title_bar = StandardTitleBar(self)
        title_bar.titleLabel.setStyleSheet("""
            QLabel{
                background: transparent;
                font-family: "Microsoft YaHei", "微软雅黑", "PingFang SC", "Hiragino Sans GB", "Segoe UI", sans-serif;
                font-size: 13px;
                padding: 0 4px
            }
        """)
        self.setTitleBar(title_bar)
        self.setWindowTitle("戴森球计划种子搜索器 made by 前前&哒哒")
        self.setWindowIcon(QIcon(r"assets\icon.png"))
        width: int = 1210
        height: int = width // 16 * 10
        self.resize(width, height)
        setTheme(Theme.AUTO)
        self.titleBar.raise_()

        self.mainLayout = VBoxLayout(self)
        

        self.setLayout(self.mainLayout)
        self.__build__()

        screen = QApplication.primaryScreen()
        screen_geometry = screen.geometry()
        x = (screen_geometry.width() - width) // 2
        y = (screen_geometry.height() - height) // 2
        self.move(x, y)

        signal.signal(signal.SIGINT, self.__handle_exit__)
        self.search_thread = SearchThread(self)
        SearchMessages.searchEnd.connect(self._on_search_finish)

    def closeEvent(self, e: QEvent):
        if self.search_thread.isRunning():
            message_box = MessageBox(
                "有搜索任务正在进行中，是否强制退出？",
                "强制退出可能会导致部分结果未保存，建议先停止搜索任务。",
                self.window(),
            )
            if message_box.exec() != 1:
                e.ignore()
                return # 取消关闭事件
            self.search_thread.terminate()
            while self.search_thread.isRunning():
                QApplication.processEvents()
            self.search_thread.deleteLater()
        return super().closeEvent(e)

    def _on_search_finish(self):
        self.button_start.setEnabled(True)
        self.button_stop.setEnabled(False)

    def __init__layout__(self):
        self.topLayout = QGridLayout()
        self.mainLayout.addLayout(self.topLayout)
        self.middleLayout = QHBoxLayout()
        self.mainLayout.addLayout(self.middleLayout, stretch=1)
        self.userLayout = UserLayout()
        self.mainLayout.addLayout(self.userLayout)
        self.buttonLayout = QHBoxLayout()
        self.mainLayout.addStretch()
        self.mainLayout.addLayout(self.buttonLayout)
        self.mainLayout.setContentsMargins(10, 50, 10, 10)

    def __init__widgets__(self):
        self.label_seed_range = BodyLabel("种子范围:")
        self.input_seed_start = LimitLineEdit("start_seed", min_value=0, max_value=99999999)
        # self.input_seed_start.setMinimumWidth(80)
        self.input_seed_end = LimitLineEdit("end_seed", min_value=0, max_value=99999999)
        # self.input_seed_end.setMinimumWidth(80)
        self.label_star_num = BodyLabel("恒星数:")
        self.input_star_num_start = LimitLineEdit("start_star_num", min_value=32, max_value=64)
        self.input_star_num_end = LimitLineEdit("end_star_num", min_value=32, max_value=64)
        self.label_batch_size = BodyLabel("批处理大小:")
        self.input_batch_size = LimitLineEdit("batch_size", min_value=1, max_value=4096)
        self.label_thread_num = BodyLabel("线程数:")
        self.input_thread_num = LimitLineEdit("max_thread", min_value=1, max_value=128)
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

    def __on_button_start_clicked(self):
        if self.search_thread.isRunning():
            log.info("搜索线程已在运行中，请勿重复点击开始按钮")
            return
        
        import math
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
