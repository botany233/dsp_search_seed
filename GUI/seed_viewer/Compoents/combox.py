from qfluentwidgets import ComboBox, setFont
from PySide6.QtWidgets import QHBoxLayout, QFrame, QGridLayout, QWidget
from PySide6.QtCore import QEvent, QRectF
from PySide6.QtGui import QPainter

from GUI import vein_names, planet_types, star_types
from CApi import resource_rate_c
from language import tr_domain

custom_types = ["例子：种子号", "例子：恒星数", "自定义1", "自定义2", "自定义3"]

sort_types = {
    "矿脉数量": vein_names,
    "矿脉储量": vein_names,
    "行星类别": planet_types,
    "恒星类别": star_types,
    "自定义": custom_types
}

sort_type_domains = {
    "矿脉数量": "veins",
    "矿脉储量": "veins",
    "行星类别": "planet_types",
    "恒星类别": "star_types",
    "自定义": "custom_sort_types",
}


def _set_min_width(combo: ComboBox, texts: list[str]) -> None:
    if not texts:
        return
    max_width = max(combo.fontMetrics().horizontalAdvance(text) for text in texts)
    combo.setMinimumWidth(max_width + 45)

class ResourceRateComboBox(ComboBox):
    def __init__(self):
        super().__init__()
        self.values = list(resource_rate_c)
        texts = [tr_domain("resource_rates", value) for value in self.values]
        self.addItems(texts)
        self.setCurrentIndex(len(resource_rate_c) - 1)
        _set_min_width(self, texts)

    def current_value(self) -> str:
        index = self.currentIndex()
        if index < 0 or index >= len(self.values):
            return self.values[-1]
        return self.values[index]

    def current_resource_index(self) -> int:
        index = self.currentIndex()
        if index < 0 or index >= len(self.values):
            return len(self.values) - 1
        return index

class MainTypeComboBox(ComboBox):
    def __init__(self):
        super().__init__()
        # self.currentIndexChanged.connect(self._on_currentIndexChanged)
        self.values = list(sort_types.keys())
        texts = [tr_domain("sort_types", value) for value in self.values]
        self.addItems(texts)
        _set_min_width(self, texts)

    def current_value(self) -> str:
        index = self.currentIndex()
        if index < 0 or index >= len(self.values):
            return self.values[0]
        return self.values[index]

class SubTypeComboBox(ComboBox):
    def __init__(self, main_comobox: MainTypeComboBox):
        super().__init__()
        self.main_comobox = main_comobox
        self.values = []
        texts = []
        for main_type, values in sort_types.items():
            domain = sort_type_domains[main_type]
            texts += [tr_domain(domain, value) for value in values]
        _set_min_width(self, texts)
        self.main_comobox.currentIndexChanged.connect(self.refresh)
        self.refresh()

    def refresh(self, *args):
        main_type = self.main_comobox.current_value()
        domain = sort_type_domains[main_type]
        self.values = sort_types[main_type]
        texts = [tr_domain(domain, value) for value in self.values]
        self.clear()
        self.addItems(texts)

    def current_value(self) -> str:
        index = self.currentIndex()
        if index < 0 or index >= len(self.values):
            return self.values[0]
        return self.values[index]
