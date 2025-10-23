from qfluentwidgets import SwitchButton
from config import cfg


class ConfigSwitchButton(SwitchButton):
    def set_config(self, config_key: str, config_obj=None) -> None:
        self.config_key = config_key
        self.config_obj = config_obj

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
            print(f"设置配置项 {self.config_key} 失败: {e}")
