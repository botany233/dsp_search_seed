from PySide6.QtGui import QResizeEvent
from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout
from PySide6.QtCore import Qt
from qfluentwidgets import (
    TitleLabel,
    BodyLabel,
    isDarkTheme,
    SmoothScrollArea,
)
from typing import Literal
from config import cfg
from logger import log

from .Compoents import LimitLineEdit
from multiprocessing import cpu_count

from GUI.Compoents import AutoFixedConfigComboBox, ConfigSwitchButton
from CApi import get_device_info_c, set_device_id_c

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

        self.mainLayout.addWidget(TitleLabel("基础设置"))

        basicSettingWidget = QWidget()
        self.mainLayout.addWidget(basicSettingWidget)
        self.basicSettingLayout = QGridLayout(basicSettingWidget)

        basicSettingwidgets = [
            BodyLabel("最大线程数:"),
            LimitLineEdit("max_thread", min_value=1, max_value=128, default_value=cpu_count(), empty_invisible=False),
        ]
        for index, widget in enumerate(basicSettingwidgets):
            self.basicSettingLayout.addWidget(widget, index//2+1, index%2)

        self.mainLayout.addWidget(TitleLabel("GPU设置"))

        GPUSettingWidget = QWidget()
        self.mainLayout.addWidget(GPUSettingWidget)
        self.GPUSettingLayout = QGridLayout(GPUSettingWidget)

        GPUSettingwidgets = [
            BodyLabel("工作组大小："),
            LocalSizeComboBox(),
            BodyLabel("设备id："),
            DeviceComboBox(),
        ]
        for index, widget in enumerate(GPUSettingwidgets):
            self.GPUSettingLayout.addWidget(widget, index//2+1, index%2)

# class BiggerBodyLabel(BodyLabel):
#     def getFont(self):
#         return getFont(18)

# class BaseBox(MessageBoxBase):
#     def __init__(self, title: str = "Base Box", parent=None):
#         super().__init__(parent)
#         # self.yesButton.setText("保存")
#         self.yesButton.setHidden(True)
#         self.cancelButton.setText("返回")

#         self.titleLabel = BiggerBodyLabel(title)
#         self.viewLayout.addWidget(self.titleLabel)
    
#         self.mainWidget = QFrame()
#         self.viewLayout.addWidget(self.mainWidget)

#         self.mainLayout = QVBoxLayout()
#         self.mainLayout.setContentsMargins(0,15,0,0)
#         self.mainLayout.setAlignment(Qt.AlignmentFlag.AlignTop)
#         self.mainWidget.setLayout(self.mainLayout)
        
#         self.mainWidget.setMinimumWidth(300)
#         self.pressedMask = False
    
#     def mousePressEvent(self, event: QMouseEvent) -> None:
#         hit_widget = self.childAt(event.position().toPoint())
#         if hit_widget is self.windowMask:
#             self.pressedMask = True
#         self.focusWidget().clearFocus() if self.focusWidget() else None
#         return super().mousePressEvent(event)
#     def mouseReleaseEvent(self, event: QMouseEvent) -> None:
#         hit_widget = self.childAt(event.position().toPoint())
#         if hit_widget is self.windowMask and self.pressedMask:
#             self.cancelButton.click()
#         self.pressedMask = False
#         return super().mouseReleaseEvent(event)


# class BaseSettingFrame(QFrame):
#     def __init__(self, title: str = "Base Setting", parent=None):
#         super().__init__(parent)

#         lt_qss = """BaseSettingFrame{border: 1px solid #ededed; border-radius: 8px;}"""
#         dk_qss = """BaseSettingFrame{border: 1px solid #2B2B2B; border-radius: 8px;}"""
#         if isDarkTheme():
#             self.setStyleSheet(dk_qss)
#         else:
#             self.setStyleSheet(lt_qss)

#         self.mainLayout = QVBoxLayout(self)
#         self.mainLayout.setContentsMargins(0, 15, 0, 15)

#         self.titleLabel = TitleLabel(title)
#         self.titleLabel.setContentsMargins(15, 0, 15, 0)
#         self.mainLayout.addWidget(self.titleLabel)
#         self.mainLayout.setAlignment(self.titleLabel, Qt.AlignmentFlag.AlignTop)
#         self.mainLayout.addSpacing(10)
#         self.setMinimumHeight(50)

# class BaseSettingItemFrame(QFrame):
#     def __init__(self, title: str = "Setting Item", parent=None, cfg_obj=None, cfg_key: str = ""):
#         super().__init__(parent)
#         self.cfg_obj = cfg_obj
#         self.cfg_key = cfg_key

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(15, 0, 15, 0)
        self.label = BodyLabel(title)
        self.mainLayout.addWidget(self.label)

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


class NormalSettingFrame(BaseSettingFrame):
    def __init__(self, title: str = "基础设置", parent=None):
        super().__init__(title, parent)
        
        self.threadNumSetting = EditLineSettingItemFrame(
            "最大线程数",
            None,
            cfg_obj=cfg.config,
            cfg_key="max_thread",
            min_value=1,
            max_value=128, 
            default_value=cpu_count(),
            empty_invisible=False
        )
        self.useGpuSetting = SwitchButtonSettingItemFrame(
            "启用GPU加速",
            None,
            cfg_key="use_gpu",
        )

        self.mainLayout.addWidget(self.threadNumSetting)
        self.mainLayout.addWidget(self.useGpuSetting)

class GPUSettingFrame(BaseSettingFrame):
    def __init__(self, title: str = "GPU设置", parent=None):
        super().__init__(title, parent)

        self.localSizeSetting = ComboBoxSettingItemFrame(
            ["32", "64", "128", "256", "512", "1024"],
            "工作组大小",
            cfg_key="local_size",
        )
        devices_info = get_device_info_c()
        self.gpuDeviceSetting = ComboBoxSettingItemFrame(
            devices_info,
            "GPU设备",
            cfg_key="device_name",
            type_input="str",
        )
        if cfg.config.device_name == "cpu":
            self.gpuDeviceSetting.comboBox.setCurrentIndex(-1)
            self.gpuDeviceSetting.comboBox.setText("不使用GPU")
        set_device_id_c(self.gpuDeviceSetting.comboBox.currentIndex())

        self.gpuDeviceSetting.comboBox.currentIndexChanged.connect(self._gpu_device_changed)

        self.mainLayout.addWidget(self.localSizeSetting)
        self.mainLayout.addWidget(self.gpuDeviceSetting)
    
    def _gpu_device_changed(self, index):
        set_device_id_c(index)

    def resizeEvent(self, event: QResizeEvent) -> None:
        width = self.width()
        self.gpuDeviceSetting.comboBox.setMaximumWidth(width*2//3)
        self.gpuDeviceSetting.comboBox.setMinimumWidth(width*2//3)
        return super().resizeEvent(event)

class SettingInterface(SmoothScrollArea):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.expanded = True

        self.scrollWidget = QFrame()
        self.setWidget(self.scrollWidget)
        self.setWidgetResizable(True)
        self.mainLayout = QHBoxLayout(self.scrollWidget)
        self.mainLayout.setContentsMargins(10, 0, 10, 0)

        self._init_left_layout()
        self._init_right_layout()

        self.mainLayout.addLayout(self.leftLayout)
        self.mainLayout.addLayout(self.rightLayout)

        self.enableTransparentBackground()

        self.GPUSetting.gpuDeviceSetting.comboBox.currentIndexChanged.connect(self._on_gpu_device_changed)
        self.normalSetting.useGpuSetting.switchButton.checkedChanged.connect(self._on_use_gpu_changed)

    def _on_gpu_device_changed(self, index):
        self.normalSetting.useGpuSetting.switchButton.setChecked(1)
    
    def _on_use_gpu_changed(self, checked):
        if not checked:
            self.GPUSetting.gpuDeviceSetting.comboBox.setCurrentIndex(-1)
            self.GPUSetting.gpuDeviceSetting.comboBox.setText("不使用GPU")
            set_device_id_c(-1)
            cfg.config.device_name = "cpu"
            cfg.save()
        else:
            if cfg.config.device_name == "cpu":
                self.GPUSetting.gpuDeviceSetting.comboBox.setCurrentIndex(0)

    def _init_left_layout(self):
        self.leftLayout = QVBoxLayout()
        self.leftLayout.setContentsMargins(10, 10, 10, 10)

        self.normalSetting = NormalSettingFrame()
        self.leftLayout.addWidget(self.normalSetting)

        self.leftLayout.addStretch()

    def _init_right_layout(self):
        self.rightLayout = QVBoxLayout()
        self.rightLayout.setContentsMargins(10, 10, 10, 10)

        self.GPUSetting = GPUSettingFrame()
        self.rightLayout.addWidget(self.GPUSetting)

        self.rightLayout.addStretch()

    def __bind_right_to_left(self):
        if not self.expanded:
            return
        self.expanded = False
        self.rightLayout.setContentsMargins(0, 0, 0, 0)
        self.rightLayout.setParent(None)
        stretch = self.leftLayout.itemAt(self.leftLayout.count() - 1)
        if stretch and stretch.spacerItem():
            self.leftLayout.removeItem(stretch)

        self.leftLayout.addLayout(self.rightLayout)
        pass

    def __rebind_right(self):
        if self.expanded:
            return
        self.expanded = True
        self.rightLayout.setContentsMargins(10, 10, 10, 10)
        self.rightLayout.setParent(None)
        self.mainLayout.addLayout(self.rightLayout)
        self.leftLayout.addStretch()
        pass

    def resizeEvent(self, event: QResizeEvent) -> None:
        width = self.width()
        if width < 800:
            self.__bind_right_to_left()
        elif not self.expanded:
            self.__rebind_right()

        return super().resizeEvent(event)