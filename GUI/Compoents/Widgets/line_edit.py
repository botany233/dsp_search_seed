__all__ = ["LabelWithLineEdit"]
from qfluentwidgets import LineEdit, BodyLabel


from config import cfg

class ConfigLineEdit(LineEdit):

    def __init__(self, parent=None, config_key: str | None = None):
        super().__init__(parent)
        self.config_key = config_key
        self.textEdited.connect(self._on_text_edited)
        text = ""
        if self.config_key:
            if self.config_key.endswith('[0]'):
                self.config_key = self.config_key[:-3]
                self.flag = 0
                text = getattr(cfg.config, self.config_key) if self.config_key else ""
                text = str(text[0])
            elif self.config_key.endswith('[1]'):
                self.config_key = self.config_key[:-3]
                self.flag = 1
                text = getattr(cfg.config, self.config_key) if self.config_key else ""
                text = str(text[1])
            else:
                self.config_key = self.config_key
                self.flag = -1
                text = getattr(cfg.config, self.config_key) if self.config_key else ""
                text = str(text)
        if text == "-1":
            text = ""
        self.setText(text)

        self._text = self.text()

    def _on_text_edited(self, text: str) -> None:
        text = text.strip()
        try:
            if self.config_key:
                if self.flag == 0:
                    itext:int = int(text) if len(text) > 0 else -1
                    value = (itext, getattr(cfg.config, self.config_key)[1])
                elif self.flag == 1:
                    itext:int = int(text) if len(text) > 0 else -1
                    value = (getattr(cfg.config, self.config_key)[0], itext)
                else:
                    itext:int = int(text) if len(text) > 0 else -1
                    value = itext

                print(f"设置配置项 {self.config_key} 为 {value}")
                self._text = text
                setattr(cfg.config, self.config_key, value)
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


