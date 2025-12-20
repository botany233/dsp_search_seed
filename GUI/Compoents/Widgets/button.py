from qfluentwidgets import SwitchButton, setFont
from config import cfg
from logger import log

class ConfigSwitchButton(SwitchButton):
    def setHideText(self, hide: bool) -> None:
        if hide:
            self.label.hide()
        else:
            self.label.show()
    def set_config(self, config_key: str, config_obj=None) -> None:
        self.config_key = config_key
        if config_obj is None:
            config_obj = cfg.config
        self.config_obj = config_obj
        qss = """
        QLabel{font-size:12px;}"""
        self.label.setStyleSheet(qss)

        value = getattr(self.config_obj, self.config_key)
        self.setChecked(bool(value))
        self.checkedChanged.connect(self._on_checked_changed)

    def _on_checked_changed(self, checked: bool) -> None:
        try:
            if self.config_obj is not None:
                setattr(self.config_obj, self.config_key, (checked))
            else:
                setattr(cfg.config, self.config_key, (checked))
            cfg.save()
        except Exception as e:
            msg = f"设置配置项 {self.config_obj}.{self.config_key} 失败: {e}" if self.config_obj is not None else f"设置配置项 {self.config_key} 失败: {e}"
            log.error(msg)