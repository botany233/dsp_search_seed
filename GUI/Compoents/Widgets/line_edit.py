from qfluentwidgets import LineEdit, BodyLabel
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
        invisible_value: str|None = None
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
        self.invisible_value = invisible_value
        self.show_cfg_text()

    def setText(self, text: str) -> None:
        show_text = "" if text == self.invisible_value else text
        return super().setText(show_text)

    def show_cfg_text(self) -> None:
        self.setText(str(getattr(self.config_obj, self.config_key)))

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
        value = self._type_convert(text)
        if value is None:
            self.setPlaceholderText("请输入有效数字")
            self.setText("")
        else:
            if self.min_value is not None and value < self.min_value:
                self.setPlaceholderText(f"最小值为{self.min_value}")
                self.setText("")
            elif self.max_value is not None and value > self.max_value:
                self.setPlaceholderText(f"最大值为{self.max_value}")
                self.setText("")
            else:
                setattr(self.config_obj, self.config_key, value)
                cfg.save()
                self.setPlaceholderText("")
                self.show_cfg_text()

class LabelWithLimitLineEdit(LimitLineEdit):
    def __init__(
        self,
        config_key: str,
        config_obj: Any = None,
        label: str = "You should give me even a foo as least",
        type_input: Literal["int"] | Literal["float"] | Literal["str"] = "int",
        min_value: int | float | None = None,
        max_value: int | float | None = None,
        invisible_value: str | None = None,
    ):
        super().__init__(
            config_key, config_obj, type_input, min_value, max_value, invisible_value
        )
        self.label_box = BodyLabel(label)
        self.setToolTip(label)
        self.label_box.setToolTip(label)
        self.hBoxLayout.insertWidget(0, self.label_box)

        self.textChanged.connect(self._shadow_label)
        QTimer.singleShot(0, self._shadow_label)

    def setShadow(self, enable: bool, extra=None) -> None:
        if enable:
            self.label_box.setStyleSheet("color: gray;")
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
        pass

