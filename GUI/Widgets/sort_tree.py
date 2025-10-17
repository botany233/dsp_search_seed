from ..Compoents import SortTree
from PySide6.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout

from qfluentwidgets import VBoxLayout, PushButton, PopUpAniStackedWidget



class SortTreeWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.tree = SortTree()

        self.tree.setBorderRadius(8)
        self.tree.setBorderVisible(True)

        self.vBoxLayout = QVBoxLayout(self)
        self.vBoxLayout.addWidget(self.tree)

        self.buttonLayout = QHBoxLayout()

        self.vBoxLayout.addLayout(self.buttonLayout)
        
        self.clearButton = PushButton("清空")
        self.clearButton.clicked.connect(self.__on_clear_button_clicked__)
        self.addItemButton = PushButton("添加项")
        self.buttonLayout.addWidget(self.clearButton)
        self.buttonLayout.addWidget(self.addItemButton)

    def add_item(self, texts):
        return self.tree.add_item(texts)

    def setBorderVisible(self, visible: bool):
        self.tree.setBorderVisible(visible)

    def setBorderRadius(self, radius: int):
        self.tree.setBorderRadius(radius)

    def __on_clear_button_clicked__(self):
        self.tree.clear()
