from PySide6.QtWidgets import QWidget, QVBoxLayout, QGridLayout, QHBoxLayout
from .line_edit import LabelWithLineEdit

from qfluentwidgets import PushButton, PopUpAniStackedWidget

class SettingsWindow(QWidget):
    def __init__(self, parent=None, items: list[str] = ["wa", "al", "aw", "la", "lw", "la"], context = None):
        super().__init__(parent)
        
        self.context = context # 预留给设置使用的上下文

        print(self.context)
        
        self.mainLayout = QVBoxLayout(self)
        
        self.settingsLayout = QGridLayout()

        self._addItems(items)


        self.mainLayout.addLayout(self.settingsLayout)

        self.mainLayout.addStretch()

        self.buttonLayout = QHBoxLayout()

        self.saveButton = PushButton("保存")
        self.cancelButton = PushButton("取消")
        self.saveButton.clicked.connect(self._save_button_clicked__)
        self.cancelButton.clicked.connect(self._cancel_button_clicked__)
        self.buttonLayout.addWidget(self.saveButton)
        self.buttonLayout.addWidget(self.cancelButton)
        self.mainLayout.addLayout(self.buttonLayout)

    def _addItems(self, items: list[str]):
        from math import sqrt
        count = len(items)
        itemsEachLine = int(sqrt(count)) + 1
        j = 0
        for i, item in enumerate(items):
            if i % itemsEachLine == 0 and i != 0:
                j += 1
            i = i % itemsEachLine
            line_edit = LabelWithLineEdit(item)
            self.settingsLayout.addWidget(line_edit, j, i)

    def _save_button_clicked__(self):
        # 在这里添加保存设置的逻辑
        parent: PopUpAniStackedWidget = self.parent() # type: ignore

        parent.setCurrentIndex(0)
        parent.removeWidget(self)
        self.deleteLater()

    def _cancel_button_clicked__(self):
        parent: PopUpAniStackedWidget = self.parent() # type: ignore

        parent.setCurrentIndex(0)
        parent.removeWidget(self)
        self.deleteLater()




if __name__ == "__main__":
    import sys
    pass