from qfluentwidgets import FluentLabelBase, BodyLabel
from PySide6.QtCore import Signal, QTimer

class AlwaysShowLabel(BodyLabel):
    
    onTextChanged = Signal()

    def _init(self):
        self.onTextChanged.connect(self._resize)
        QTimer.singleShot(0, self._resize)
        return super()._init()

    def setText(self, arg__1: str) -> None:
        self.onTextChanged.emit()
        return super().setText(arg__1)
    
    def _resize(self):

        length = self.fontMetrics().horizontalAdvance(self.text()) + 10
        self.setMinimumWidth(length)
        width = self.fontMetrics().horizontalAdvance(self.text()) + 10
        self.setFixedWidth(width)
