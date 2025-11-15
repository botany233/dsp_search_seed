from PySide6.QtWidgets import QFrame, QVBoxLayout
from PySide6.QtCore import Qt, QUrl, QSize
from PySide6.QtGui import QResizeEvent
from qfluentwidgets import FluentIcon, HyperlinkLabel, LargeTitleLabel, TitleLabel, BodyLabel

from config import cfg

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
        self.mainLayout.addStretch(1)

    def resizeEvent(self, event: QResizeEvent) -> None:
        top = max(event.size().height()//2 - 128, 0)
        self.mainLayout.setContentsMargins(0, top, 0, 0)
        return super().resizeEvent(event)
