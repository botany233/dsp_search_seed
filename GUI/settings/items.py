from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout
from PySide6.QtCore import Qt
from qfluentwidgets import TitleLabel, BodyLabel, isDarkTheme, ComboBox, MessageBox
from typing import Literal
from config import cfg
from logger import log
from language import get_languages, tr
from GUI.Compoents import LimitLineEdit, AutoFixedConfigComboBox, ConfigSwitchButton

class BaseSettingFrame(QFrame):
    def __init__(self, title: str = "Base Setting", parent=None):
        super().__init__(parent)


        lt_qss = """
        BaseSettingFrame{border: 4px solid #ededed; border-radius: 10px;}
        """
        dk_qss = """
        BaseSettingFrame{border: 4px solid #2B2B2B; border-radius: 10px;}
        """
        if isDarkTheme():
            self.setStyleSheet(dk_qss)
        else:
            self.setStyleSheet(lt_qss)

        self.mainLayout = QVBoxLayout(self)
        self.mainLayout.setContentsMargins(0, 15, 0, 15)
        self.mainLayout.setAlignment(Qt.AlignmentFlag.AlignLeft)

        self.titleLabel = TitleLabel(title)
        self.titleLabel.setContentsMargins(15, 0, 15, 0)
        self.mainLayout.addWidget(self.titleLabel)
        self.mainLayout.setAlignment(self.titleLabel, Qt.AlignmentFlag.AlignTop)
        self.mainLayout.addSpacing(10)

class BaseSettingItemFrame(QFrame):
    def __init__(self, title: str = "Setting Item", parent=None, cfg_obj=None, cfg_key: str = ""):
        super().__init__(parent)
        self.cfg_obj = cfg_obj
        self.cfg_key = cfg_key

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(15, 0, 15, 0)
        self.labelLayout = QHBoxLayout()
        self.labelLayout.setContentsMargins(0, 0, 0, 0)
        self.labelLayout.setAlignment(Qt.AlignmentFlag.AlignVCenter)
        self.label = BodyLabel(title)
        self.labelLayout.addWidget(self.label)
        self.mainLayout.addLayout(self.labelLayout)

        self.setMinimumHeight(40)
        self.mainLayout.addStretch()

    def _cfg_save(self, value):
        try:
            if self.cfg_obj and self.cfg_key:
                setattr(self.cfg_obj, self.cfg_key, value)
                cfg.save()
        except Exception as e:
            log.error(f"设置配置项 {self.cfg_obj} 失败: {e}")
    
    def _cfg_load(self):
        try:
            if self.cfg_obj and self.cfg_key:
                return getattr(self.cfg_obj, self.cfg_key)
        except Exception as e:
            log.error(f"获取配置项 {self.cfg_obj} 失败: {e}")
        return None


class EditLineSettingItemFrame(BaseSettingItemFrame):
    def __init__(
        self,
        title: str = "Setting Item",
        parent=None,
        cfg_obj=None,
        cfg_key: str = "",
        type_input: Literal["int", "float", "str"] = "int",
        min_value: int|float|None = None,
        max_value: int|float|None = None,
        default_value: int|float|str = "",
        empty_invisible: bool = True,
    ):
        super().__init__(title, parent, cfg_obj, cfg_key)

        self.line = LimitLineEdit(cfg_key, cfg_obj, type_input=type_input, empty_invisible=empty_invisible, min_value=min_value, max_value=max_value, default_value=default_value)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.setAlignment(self.line, Qt.AlignmentFlag.AlignRight)


class ComboBoxSettingItemFrame(BaseSettingItemFrame):
    def __init__(
        self,
        items: list[str],
        title: str = "Setting Item",
        parent=None,
        cfg_obj=None,
        cfg_key: str = "",
        type_input: Literal["int", "float", "str"] = "int",
    ):
        super().__init__(title, parent, cfg_obj, cfg_key)

        self.comboBox = AutoFixedConfigComboBox(config_key=cfg_key, type_input=type_input, config_obj=cfg_obj, items=items)
        self.mainLayout.addWidget(self.comboBox)
        self.mainLayout.setAlignment(self.comboBox, Qt.AlignmentFlag.AlignRight)

    def addItems(self, items: list[str]):
        self.comboBox.addItems(items)

class SwitchButtonSettingItemFrame(BaseSettingItemFrame):
    def __init__(self, title: str = "Setting Item", parent=None, cfg_obj=None, cfg_key: str = ""):
        super().__init__(title, parent, cfg_obj, cfg_key)

        self.switchButton = ConfigSwitchButton()
        self.switchButton.set_config(cfg_key, cfg_obj)
        self.switchButton.setHideText(True)
        self.mainLayout.addWidget(self.switchButton)
        self.mainLayout.setAlignment(self.switchButton, Qt.AlignmentFlag.AlignRight)


class LanguageSettingItemFrame(BaseSettingItemFrame):
    def __init__(self, title: str, parent=None):
        super().__init__(title, parent, cfg.config, "language")

        self.languages = get_languages()
        if not self.languages:
            self.languages = [{"code": cfg.config.language, "native_name": cfg.config.language}]

        self.comboBox = ComboBox()
        self.comboBox.addItems([language["native_name"] for language in self.languages])

        self.language_codes = [language["code"] for language in self.languages]
        try:
            current_index = self.language_codes.index(cfg.config.language)
        except ValueError:
            current_index = 0
        self.comboBox.setCurrentIndex(current_index)
        self._resize_combo_box()

        self.mainLayout.addWidget(self.comboBox)
        self.mainLayout.setAlignment(self.comboBox, Qt.AlignmentFlag.AlignRight)
        self.comboBox.currentIndexChanged.connect(self._on_language_changed)

    def _resize_combo_box(self):
        font_metrics = self.comboBox.fontMetrics()
        max_width = max(font_metrics.horizontalAdvance(language["native_name"]) for language in self.languages)
        self.comboBox.setMinimumWidth(max_width + 45)

    def _on_language_changed(self, index: int):
        if index < 0 or index >= len(self.language_codes):
            return

        language_code = self.language_codes[index]
        if cfg.config.language == language_code:
            return

        cfg.config.language = language_code
        cfg.save()

        message_box = MessageBox(
            tr("settings.language.restart_title", language_code),
            tr("settings.language.restart_content", language_code),
            self.window(),
        )
        message_box.yesButton.setText(tr("common.confirm", language_code))
        message_box.cancelButton.hide()
        message_box.exec()


