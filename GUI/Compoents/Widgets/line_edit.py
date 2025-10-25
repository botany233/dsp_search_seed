from qfluentwidgets import LineEdit, BodyLabel
from PySide6.QtCore import QTimer
from typing import Any, Literal
from config import cfg


class ConfigLineEdit(LineEdit):
    def __init__(
        self,
        parent=None,
        config_key: str | None = None,
        config_obj=None,
        type_input: str = "int",
    ):
        super().__init__(parent)
        self.config_key = config_key
        self.type_input = type_input
        self.config_obj = config_obj
        if self.config_obj is None:
            self.config_obj = cfg.config
        self.textEdited.connect(self._on_text_edited)
        text = ""
        if self.config_key:
            if self.config_key.endswith("[0]"):
                self.config_key = self.config_key[:-3]
                self.flag = 0
                text = (
                    getattr(self.config_obj, self.config_key) if self.config_key else ""
                )
                text = str(text[0])
            elif self.config_key.endswith("[1]"):
                self.config_key = self.config_key[:-3]
                self.flag = 1
                text = (
                    getattr(self.config_obj, self.config_key) if self.config_key else ""
                )
                text = str(text[1])
            else:
                self.config_key = self.config_key
                self.flag = -1
                text = (
                    getattr(self.config_obj, self.config_key) if self.config_key else ""
                )
                text = str(text)

        if text == "-1" or text == "-1.0":
            text = ""
        self.setText(text)

        self._text = self.text()

    def setText(self, arg__1: str | None) -> None:
        self._text = arg__1 if arg__1 is not None else ""
        return super().setText(arg__1)

    def _type_convert__(self, text: str):
        if self.type_input == "int":
            return int(text) if len(text) > 0 else -1
        elif self.type_input == "float":
            return float(text) if len(text) > 0 else -1.0
        else:
            return text

    def _on_text_edited(self, text: str) -> None:
        text = text.strip()
        try:
            if self.config_key:
                if self.flag == 0:
                    itext = self._type_convert__(text)
                    value = (itext, getattr(cfg.config, self.config_key)[1])
                elif self.flag == 1:
                    itext = self._type_convert__(text)
                    value = (getattr(cfg.config, self.config_key)[0], itext)
                else:
                    itext = self._type_convert__(text)
                    value = itext
                self._text = text
                setattr(self.config_obj, self.config_key, value)
                self.setPlaceholderText("")
                cfg.save()

        except Exception:
            self.setPlaceholderText("请输入有效数字")
            self.setText(self._text)

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

class LabelWithLineEdit(ConfigLineEdit):
    def __init__(
        self,
        label: str = "You should give me even a foo as least",
        parent=None,
        config_key: str | None = None,
    ):
        super().__init__(parent, config_key=config_key)
        self.label_box = BodyLabel(label)
        self.setToolTip(label)
        self.label_box.setToolTip(label)
        self.hBoxLayout.insertWidget(0, self.label_box)
        self._text = self.text()
        self.textChanged.connect(self._shadow_label)
        QTimer.singleShot(0, self._shadow_label)

    def SetEnableVerify(self) -> None:
        self.textEdited.connect(self._on_text_edited_verify)

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

    def _on_text_edited_verify(self, text: str) -> None:
        text = text.strip()
        if len(text) == 0:
            self.setPlaceholderText("")
            self._text = text
            return
        try:
            itext: int = int(text)
            self.setPlaceholderText("")
            self._text = text
        except Exception:
            self.setPlaceholderText("请输入有效数字")
            self.setText(self._text)

        self._shadow_label()
