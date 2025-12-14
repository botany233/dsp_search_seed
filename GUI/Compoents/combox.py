from qfluentwidgets import ComboBox, BodyLabel, setFont
from PySide6.QtWidgets import QHBoxLayout, QFrame, QGridLayout, QWidget
from PySide6.QtCore import QEvent, QRectF
from PySide6.QtGui import QPainter
from CApi import *

from config import cfg

class LocalSizeComboBox(ComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)

        setFont(self, 12)
        for i in [32, 64, 128, 256, 512, 1024]:
            self.addItem(str(i))
        self.fresh(cfg.config.local_size)

        self.currentIndexChanged.connect(self._on_currentIndexChanged)

    def fresh(self, local_size: int) -> int:
        set_local_size_c(local_size)
        real_local_size = get_local_size_c()
        index = self.findText(str(real_local_size))
        if index != -1:
            self.setCurrentIndex(index)
        else:
            self.setCurrentIndex(3)  # 默认256
        return real_local_size

    def _on_currentIndexChanged(self, index: int):
        local_size = self.fresh(int(self.currentText()))
        cfg.config.local_size = local_size
        cfg.save()

class DeviceComboBox(ComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        setFont(self, 12)
        device_id, devices_info = self.fresh_device(cfg.config.device_name)
        self.addItem("cpu")
        for device_info in devices_info:
            self.addItem(device_info)
        self.fresh_item(device_id, devices_info)

        self.currentTextChanged.connect(self._on_currentTextChanged)

    def fresh_item(self, device_id: int, devices_info: list[str]) -> None:
        if (["cpu"] + devices_info)[device_id + 1] != self.currentText():
            self.setCurrentIndex(device_id + 1)

    def fresh_device(self, device_name: str) -> tuple[int, list[str]]:
        devices_info = get_device_info_c()
        for index, device_info in enumerate(devices_info):
            if device_info == device_name:
                new_device_id = index
                break
        else:
            new_device_id = -1
        set_device_id_c(new_device_id)
        real_device_id = get_device_id_c()
        if real_device_id < 0:
            cfg.config.device_name = "cpu"
        else:
            cfg.config.device_name = devices_info[real_device_id]
        cfg.save()
        return real_device_id, devices_info

    def _on_currentTextChanged(self, current_text: str):
        device_id, devices_info = self.fresh_device(current_text)
        self.fresh_item(device_id, devices_info)

class AutoFixedComboBox(ComboBox):
    def __init__(self, parent=None, config_key: str | None = None):
        super().__init__(parent)
        self.currentIndexChanged.connect(self._on_currentIndexChanged)
        self.config_key = config_key
        setFont(self, 12)

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
