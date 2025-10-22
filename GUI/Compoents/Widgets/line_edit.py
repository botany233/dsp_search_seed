__all__ = ["LabelWithLineEdit"]
from qfluentwidgets import LineEdit, BodyLabel


from config import cfg

class ConfigLineEdit(LineEdit):

    def __init__(self, parent=None, config_key: str | None = None, config_obj = None, type_input: str = "int"):
        super().__init__(parent)
        self.config_key = config_key
        self.type_input = type_input
        self.config_obj = config_obj
        if self.config_obj is None:
            self.config_obj = cfg.config
        self.textEdited.connect(self._on_text_edited)
        text = ""
        if self.config_key:
            if self.config_key.endswith('[0]'):
                self.config_key = self.config_key[:-3]
                self.flag = 0
                text = getattr(self.config_obj, self.config_key) if self.config_key else ""
                text = str(text[0])
            elif self.config_key.endswith('[1]'):
                self.config_key = self.config_key[:-3]
                self.flag = 1
                text = getattr(self.config_obj, self.config_key) if self.config_key else ""
                text = str(text[1])
            else:
                self.config_key = self.config_key
                self.flag = -1
                text = getattr(self.config_obj, self.config_key) if self.config_key else ""
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

        except Exception as e:
            self.setPlaceholderText("请输入有效数字")
            self.setText(self._text)



class LabelWithLineEdit(ConfigLineEdit):
    def __init__(self, label: str = "You should give me even a foo as least", parent=None, config_key: str | None = None):
        super().__init__(parent, config_key=config_key)
        self.label_box = BodyLabel(label)
        self.hBoxLayout.insertWidget(0, self.label_box)
        self._text = self.text()

    def SetEnableVerify(self) -> None:
        self.textEdited.connect(self._on_text_edited_verify)

    def _on_text_edited_verify(self, text: str) -> None:
        text = text.strip()
        if len(text) == 0:
            self.setPlaceholderText("")
            self._text = text
            return
        try:
            itext:int = int(text)
            self.setPlaceholderText("")
            self._text = text
        except Exception as e:
            self.setPlaceholderText("请输入有效数字")
            self.setText(self._text)
    



