__all__ = ["ConfigCheckBox"]
from PySide6.QtGui import QMouseEvent
from PySide6.QtWidgets import QStyleOptionButton, QStyle, QCheckBox
from qfluentwidgets import CheckBox
from config import cfg
from config.cfg_dict_tying import BaseModel


class ConfigCheckBox(CheckBox):
    def __init__(
        self,
        text: str = "",
        parent=None,
        *,
        config_obj: BaseModel | None,
        config_key: str,
    ):
        super().__init__(parent)
        if text:
            self.setText(text)
        if config_obj is None:
            config_obj = cfg.config

        self.config_obj = config_obj
        self.config_key = config_key
        self.stateChanged.connect(self._on_stateChanged)
        self.load_config()

    def _on_stateChanged(self, state: int):
        if self.config_key is None:
            return
        setattr(self.config_obj, self.config_key, self.isChecked())
        cfg.save()

    def load_config(self) -> None:
        if self.config_key is None:
            return
        config_value = getattr(self.config_obj, self.config_key)
        self.setChecked(config_value)

    def mousePressEvent(self, e):
        pos = e.position()
        opt = QStyleOptionButton()
        opt.initFrom(self)
        rect = self.style().subElementRect(QStyle.SE_CheckBoxIndicator, opt, self)
        if rect.contains(pos.toPoint()):
            self.isPressed = True
            e.accept()
        else:
            e.ignore()
        return QCheckBox.mousePressEvent(self, e)

    def mouseReleaseEvent(self, e: QMouseEvent):
        if e.type() == QMouseEvent.Type.MouseButtonRelease:
            pos = e.position()
            opt = QStyleOptionButton()
            opt.initFrom(self)
            rect = self.style().subElementRect(QStyle.SE_CheckBoxIndicator, opt, self)
            if rect.contains(pos.toPoint()):
                self.toggle()
                e.accept()
            else:
                e.ignore()
            self.isPressed = False
            return
        return super().mouseReleaseEvent(e)