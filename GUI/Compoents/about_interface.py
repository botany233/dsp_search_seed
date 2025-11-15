from PySide6.QtWidgets import QFrame, QVBoxLayout
from PySide6.QtCore import Qt, QUrl, QSize, QThread, Signal
from PySide6.QtGui import QResizeEvent
from qfluentwidgets import FluentIcon, HyperlinkLabel, LargeTitleLabel, TitleLabel, BodyLabel, HyperlinkButton, setCustomStyleSheet

import requests
from config import cfg
from logger import log

class AboutInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("AboutInterface")

        self.mainLayout = QVBoxLayout(self)
        self.mainLayout.setSpacing(0)
        self.bigIcon = HyperlinkLabel()
        self.bigIcon.setToolTip("这是链接~~")
        self.bigIcon.setIcon(FluentIcon.GITHUB.icon())
        self.bigIcon.setIconSize(QSize(128, 128))
        self.bigIcon.setUrl(QUrl("https://github.com/botany233/dsp_search_seed"))
        self.mainLayout.addWidget(self.bigIcon, alignment=Qt.AlignCenter)
        self.mainLayout.addSpacing(10)
        self.titleLabel = LargeTitleLabel("戴森球计划种子搜索查找器")

        self.mainLayout.addWidget(self.titleLabel, alignment=Qt.AlignCenter)
        self.mainLayout.addSpacing(10)
        self.authorLabel = TitleLabel("----前前&哒哒")
        self.mainLayout.addWidget(self.authorLabel, alignment=Qt.AlignCenter)
        
        self.versionLabel = BodyLabel(f"{cfg.version}")
        self.mainLayout.addWidget(self.versionLabel, alignment=Qt.AlignCenter)
        
        url = "https://github.com/botany233/dsp_search_seed/releases/latest"
        self.updateButton = HyperlinkButton(url, "新版本就绪!")
        self.updateButton.hide()
        qss = """HyperlinkButton:hover {background: transparent;}"""
        setCustomStyleSheet(self.updateButton, qss, qss)

        self.mainLayout.addWidget(self.updateButton, alignment=Qt.AlignCenter)

        self.mainLayout.addStretch(1)
        self.updateThread = UpdateThread(self)
        self.updateThread.start()
        self.updateThread.updateSignal.connect(self.on_update_found)

    def on_update_found(self, new_version: str) -> None:
        self.updateButton.setText(f"新版本 {new_version} 就绪! ")
        self.updateButton.show()
    def resizeEvent(self, event: QResizeEvent) -> None:
        top = max(event.size().height()//2 - 128, 0)
        self.mainLayout.setContentsMargins(0, top, 0, 0)
        return super().resizeEvent(event)

class UpdateThread(QThread):

    updateSignal = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.running = False

    def run(self) -> None:
        if self.running:
            return
        self.running = True
        if not cfg.version.lower().startswith("v"):
            self.running = False
            log.debug("当前为开发版本，跳过更新检查")
            return

        try:
            url = "https://api.github.com/repos/botany233/dsp_search_seed/releases/latest"

            resp = requests.get(url, allow_redirects=False)
            tag = resp.json().get("tag_name", "unknown")
            if self.compare_version(tag):
                self.updateSignal.emit(tag)

        except Exception as e:
            log.error(f"检查更新时出现错误: {e}")
        finally:
            self.running = False

    def compare_version(self, new_version: str) -> bool:
        new_version = new_version.lower()
        current_version = cfg.version.lower()
        if not new_version.startswith("v"):
            log.warning(f"新版本号格式错误: {new_version}")
            return False

        version_num = list(map(int, new_version.strip("v").split(".")))
        current_version_num = list(map(int, current_version.strip("v").split(".")))
        if len(version_num) <= 0 or len(current_version_num) <= 0:
            log.warning(f"版本号格式错误: new_version={new_version}, current_version={current_version}")
            return False
        for i in range(max(len(version_num), len(current_version_num))):
            v1 = version_num[i] if i < len(version_num) else 0
            v2 = current_version_num[i] if i < len(current_version_num) else 0
            if v1 > v2:
                return True
            elif v1 < v2:
                return False
        return False

    def terminate(self) -> None:
        self.running = False
        return super().terminate()