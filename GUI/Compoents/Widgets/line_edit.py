from qfluentwidgets import LineEdit, BodyLabel, isDarkTheme, ToolTipFilter
from PySide6.QtCore import QTimer
from typing import Any, Literal
from config import cfg

class LimitLineEdit(LineEdit):
    def __init__(
        self,
        config_key: str,
        config_obj: Any = None,
        type_input: Literal["int", "float", "str"] = "int",
        min_value: int|float|None = None,
        max_value: int|float|None = None,
        default_value: int|float|str = "",
        empty_invisible: bool = True,
    ):
        super().__init__()
        assert type_input != "str" or min_value is None and max_value is None
        self.config_key = config_key
        self.type_input = type_input
        if config_obj is None:
            self.config_obj = cfg.config
        else:
            self.config_obj = config_obj
        self.editingFinished.connect(self._on_text_edited)
        self.min_value = min_value
        self.max_value = max_value
        self.default_value = default_value
        self.empty_invisible = empty_invisible

        self.fresh()

    def fresh(self) -> None:
        config_value = getattr(self.config_obj, self.config_key)
        if config_value == self.default_value and self.empty_invisible:
            self.setText("")
        else:
            self.setText(str(config_value))

    def _type_convert(self, text: str):
        try:
            if self.type_input == "int":
                return int(text)
            elif self.type_input == "float":
                return float(text)
            else:
                return text
        except Exception:
            return None

    def _on_text_edited(self) -> None:
        text = self.text().strip()
        if text == "":
            if self.empty_invisible:
                self.setPlaceholderText("")
                self.setText("")
            else:
                self.setPlaceholderText("")
                self.setText(str(self.default_value))
            setattr(self.config_obj, self.config_key, self.default_value)
            cfg.save()
            return

        value = self._type_convert(text)
        if value is None:
            self.setPlaceholderText("请输入有效数字")
            self.setText("")
        elif self.min_value is not None and value < self.min_value:
            self.setPlaceholderText(f"最小值为{self.min_value}")
            self.setText("")
        elif self.max_value is not None and value > self.max_value:
            self.setPlaceholderText(f"最大值为{self.max_value}")
            self.setText("")
        else:
            self.setPlaceholderText("")
            if value == self.default_value and self.empty_invisible:
                self.setText("")
            else:
                self.setText(str(value))
            setattr(self.config_obj, self.config_key, value)
            cfg.save()

class LabelWithLimitLineEdit(LimitLineEdit):
    def __init__(
        self,
        config_key: str,
        config_obj: Any = None,
        label: str = "You should give me even a foo as least",
        type_input: Literal["int"] | Literal["float"] | Literal["str"] = "int",
        min_value: int | float | None = None,
        max_value: int | float | None = None,
        default_value: int|float|str = "",
        empty_invisible: bool = True,
    ):
        super().__init__(config_key, config_obj, type_input, min_value, max_value, default_value, empty_invisible)
        self.label_box = BodyLabel(label)
        self.setToolTip(label)
        self.label_box.setToolTip(label)
        self.installEventFilter(ToolTipFilter(self, showDelay=0))
        self.label_box.installEventFilter(ToolTipFilter(self.label_box, showDelay=0))
        self.hBoxLayout.insertWidget(0, self.label_box)

        self.textChanged.connect(self._shadow_label)
        QTimer.singleShot(0, self._shadow_label)

    def setShadow(self, enable: bool, extra=None) -> None:
        if enable:
            self.label_box.setStyleSheet("color: gray;")
        elif isDarkTheme():
            self.label_box.setStyleSheet("color: white;")
        else:
            self.label_box.setStyleSheet("color: black;")
        pass

    def _shadow_label(self) -> None:
        text_width = self.fontMetrics().horizontalAdvance(self.text())
        label_width = self.fontMetrics().horizontalAdvance(self.label_box.text())
        width = self.width()
        if text_width + label_width + 15 > width:
            self.setShadow(True)
        else:
            self.setShadow(False)
