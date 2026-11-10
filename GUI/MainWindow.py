import signal

from PySide6.QtCore import QEvent
from PySide6.QtWidgets import QApplication
from qfluentwidgets import (
    FluentWindow,
    Theme,
    qconfig,
    setTheme,
    setThemeColor,
    MessageBox,
    FluentIcon,
    NavigationDisplayMode,
    setCustomStyleSheet,
)

from config import cfg
from language import tr

from .search.interface import SearchInterface
from .Widgets import WaitRing
from .about import AboutInterface
from .seed_viewer.MainInterface import ViewerInterface
from .tutorial import TutorialInterface
from .settings.interface import SettingInterface
from GUI.dsp_icons import AppIcons

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
        self.setWindowTitle(tr("search.window_title"))
        self.setWindowIcon(AppIcons.LOGO.icon())
        # width: int = round(1320 / cfg.config.ui_scale_factor)
        # height: int = round(width * 0.5)
        width: int = round(1320 / cfg.config.ui_scale_factor)
        height: int = round(width * 0.5)
        self.resize(width, height)

        lt_qss = """StackedWidget{background-color: #FCFCFC;}"""
        dk_qss = """StackedWidget{background-color: #181818;}"""
        setCustomStyleSheet(self.stackedWidget, lt_qss, dk_qss)

        self.setMicaEffectEnabled(True)
        self.titleBar.raise_()

        self.navigationInterface.setExpandWidth(233)
        self.navigationInterface.displayModeChanged.connect(self._on_display_mode_changed)

        self.searchInterface = SearchInterface(self)
        self.viewerInterface = ViewerInterface(self)
        self.viewerInterface.setObjectName("viewerLayout")
        self.tutorialInterface = TutorialInterface()
        self.tutorialInterface.setObjectName("tutorialLayout")
        self.settingInterface = SettingInterface()
        self.settingInterface.setObjectName("settingLayout")
        self.aboutInterface = AboutInterface(self)
        self.aboutInterface.setObjectName("aboutLayout")

        self.addSubInterface(self.searchInterface, icon=FluentIcon.SEARCH_MIRROR, text=tr("search.navigation.title"))
        self.addSubInterface(self.viewerInterface, icon=FluentIcon.VIEW, text=tr("viewer.navigation.title"))
        self.addSubInterface(self.tutorialInterface, icon=FluentIcon.HELP, text=tr("navigation.tutorial"))
        self.addSubInterface(self.settingInterface, icon=FluentIcon.SETTING, text=tr("navigation.settings"))
        self.addSubInterface(self.aboutInterface, icon=FluentIcon.INFO, text=tr("navigation.about"))

        screen = QApplication.primaryScreen()
        screen_geometry = screen.geometry()
        x = (screen_geometry.width() - width) // 2
        y = (screen_geometry.height() - height) // 2
        self.move(x, y)

        signal.signal(signal.SIGINT, self.__handle_exit__)

        self.waiting_ring = WaitRing(self.window())
        self.waiting_ring.stop()

    def _waiting_thread_finish(self, thread) -> bool:
        message_box = MessageBox(
            tr("search.main.running_task_title"),
            tr("search.main.running_task_content"),
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
        search_task = self.searchInterface.get_search_task()
        if search_task.isRunning():
            if self._waiting_thread_finish(search_task):
                e.ignore()
                return
        elif self.viewerInterface.sort_thread.isRunning():
            if self._waiting_thread_finish(self.viewerInterface.sort_thread):
                e.ignore()
                return
        self.viewerInterface.get_data_manager.shutdown()
        return super().closeEvent(e)

    def __handle_exit__(self, signum, frame):
        import sys

        self.close()
        sys.exit(0)

    def _on_display_mode_changed(self, mode):
        if mode == NavigationDisplayMode.MENU:
            self.titleBar.titleLabel.setHidden(True)
        else:
            self.titleBar.titleLabel.setHidden(False)

if __name__ == "__main__":
    import sys

    from PySide6.QtWidgets import QApplication

    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
