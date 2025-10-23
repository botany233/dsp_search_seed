from qfluentwidgets import ComboBox, BodyLabel
from PySide6.QtWidgets import QHBoxLayout, QFrame, QGridLayout, QWidget
from PySide6.QtCore import QEvent, QRectF
from PySide6.QtGui import QPainter

from config import cfg


class AutoFixedComboBox(ComboBox):
    def __init__(self, parent=None, config_key: str | None = None):
        super().__init__(parent)
        self.currentIndexChanged.connect(self._on_currentIndexChanged)
        self.config_key = config_key

    def addItems(self, texts):
        for text in texts:
            self.addItem(text)

        font_metrics = self.fontMetrics()
        # 使用列表推导式找到最大宽度
        max_width = max(font_metrics.horizontalAdvance(text) for text in texts)
        # 添加边距
        max_width += 45

        self.setMinimumWidth(int(max_width))

    def _on_currentIndexChanged(self, index: int):
        if self.config_key is not None:
            self.change_config(self.currentText())

    def load_config(self) -> None:
        if self.config_key is None:
            return
        if hasattr(self.parent(), "config_obj"):
            config_obj = self.parent().config_obj
            config_value = getattr(config_obj, self.config_key)
            index = self.findText(config_value)
            if index != -1:
                self.setCurrentIndex(index)
        pass

    def change_config(self, config_value: str) -> None:
        if self.config_key is None:
            return
        if hasattr(self.parent(), "config_obj"):
            config_obj = self.parent().config_obj
            setattr(config_obj, self.config_key, config_value)
            cfg.save()
        pass


class LabelWithComboBox(QFrame):
    def __init__(
        self, label: str = "You should give me even a foo as least", parent=None
    ):
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
