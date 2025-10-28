from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout

from PySide6.QtCore import Qt

from qfluentwidgets import TitleLabel


class ViewerInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)

        qss = """
        QFrame {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 4px;
        }
        """
        self.setStyleSheet(qss)

        self.mainLayout = QHBoxLayout(self)

        self.leftLayout = QVBoxLayout()

        self.middleLayout = QVBoxLayout()

        self.rightLayout = QVBoxLayout()

        self.mainLayout.addLayout(self.leftLayout)
        self.mainLayout.addLayout(self.middleLayout)
        self.mainLayout.addLayout(self.rightLayout)

        left = TitleLabel("左侧区域")
        left.setAlignment(Qt.AlignCenter)
        middle = TitleLabel("中间区域")
        middle.setAlignment(Qt.AlignCenter)
        right = TitleLabel("右侧区域")
        right.setAlignment(Qt.AlignCenter)

        self.leftLayout.addWidget(left)
        self.middleLayout.addWidget(middle)
        self.rightLayout.addWidget(right)
