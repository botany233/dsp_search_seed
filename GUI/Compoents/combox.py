from qfluentwidgets import ComboBox, BodyLabel
from PySide6.QtWidgets import QHBoxLayout, QFrame, QGridLayout, QWidget
from PySide6.QtCore import QEvent, QRectF
from PySide6.QtGui import QPainter

class LabelWithComboBox(QFrame):
    def __init__(self, label: str = "You should give me even a foo as least", parent=None):
        super().__init__(parent)
        self.label_box = BodyLabel(label)
        # self.label_box.setMinimumWidth(100)
        self.comboBox = ComboBox()
        self.hBoxLayout = QGridLayout(self)
        self.hBoxLayout.addWidget(self.label_box, 0, 0)
        self.hBoxLayout.addWidget(self.comboBox, 0, 1)

    def addItems(self, items):
        for text in items:
            self.comboBox.addItem(text)
        font_metrics = self.comboBox.fontMetrics()
        # 使用列表推导式找到最大宽度
        max_width = max(font_metrics.horizontalAdvance(text) for text in items)
        # 添加边距
        max_width += 55
        
        self.comboBox.setMinimumWidth(int(max_width))