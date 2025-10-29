from qfluentwidgets import ComboBox, setFont
from PySide6.QtWidgets import QHBoxLayout, QFrame, QGridLayout, QWidget
from PySide6.QtCore import QEvent, QRectF
from PySide6.QtGui import QPainter


from GUI import vein_names, planet_types, star_types
custom_types = ["例子：种子号", "例子：恒星数", "自定义1", "自定义2", "自定义3"]

sort_types = {
    "矿物": vein_names,
    "行星类别": planet_types,
    "恒星类别": star_types,
    "自定义": custom_types
}

class MainTypeComboBox(ComboBox):
    def __init__(self):
        super().__init__()
        # self.currentIndexChanged.connect(self._on_currentIndexChanged)
        texts = sort_types.keys()
        self.addItems(texts)
        max_width = max(self.fontMetrics().horizontalAdvance(text) for text in texts)
        self.setMinimumWidth(max_width + 45)

class SubTypeComboBox(ComboBox):
    def __init__(self, main_comobox: MainTypeComboBox):
        super().__init__()
        self.main_comobox = main_comobox
        texts = []
        for i in sort_types.values():
            texts += i
        max_width = max(self.fontMetrics().horizontalAdvance(text) for text in texts)
        self.setMinimumWidth(max_width + 45)
        self.main_comobox.currentIndexChanged.connect(self.refresh)
        self.refresh()

    def refresh(self, *args):
        texts = sort_types[self.main_comobox.currentText()]
        self.clear()
        self.addItems(texts)
