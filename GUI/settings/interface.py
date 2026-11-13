from PySide6.QtGui import QResizeEvent, QColor
from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout
from PySide6.QtCore import Qt
from qfluentwidgets import SmoothScrollArea, ToolTipFilter, PixmapLabel, FluentIcon, PushButton
from config import cfg
from language import tr
from multiprocessing import cpu_count
from CApi import get_device_info_c, set_device_id_c, get_device_id_c, set_gpu_max_worker_c

from .items import (
    BaseSettingFrame,
    BaseSettingItemFrame,
    EditLineSettingItemFrame,
    ComboBoxSettingItemFrame,
    SwitchButtonSettingItemFrame,
    LanguageSettingItemFrame,
)
from .gpu_benchmark import GPUBenchmarkMessageBox

class NormalSettingFrame(BaseSettingFrame):
    def __init__(self, title: str | None = None, parent=None):
        super().__init__(title or tr("settings.basic.title"), parent)
        
        self.threadNumSetting = EditLineSettingItemFrame(
            tr("settings.basic.max_cpu_threads"),
            None,
            cfg_obj=cfg.config,
            cfg_key="max_thread",
            min_value=1,
            max_value=128, 
            default_value=cpu_count(),
            empty_invisible=False
        )
        self.languageSetting = LanguageSettingItemFrame(
            tr("settings.basic.language"),
        )
        self.nameLanguageSetting = ComboBoxSettingItemFrame(
            ["English", "中文"],
            tr("settings.basic.name_language"),
            cfg_key="name_language",
            type_input="str",
        )
        self.nameLanguageSetting.comboBox.setItemText(1, tr("meta.native_name", "zh_CN"))
        self.nameLanguageSetting.comboBox.setMinimumWidth(self.languageSetting.comboBox.minimumWidth())
        self.useGpuSetting = SwitchButtonSettingItemFrame(
            tr("settings.basic.use_gpu"),
            None,
            cfg_key="use_gpu",
        )

        self.warningLabel = PixmapLabel()
        self.threadNumSetting.labelLayout.addWidget(self.warningLabel)
        self.warningLabel.setPixmap(FluentIcon.INFO.icon().pixmap(16, 16))
        self.warningLabel.setFixedSize(16, 16)
        self.warningLabel.installEventFilter(ToolTipFilter(self.warningLabel, showDelay=0))
        self.warningLabel.setToolTip(tr("settings.basic.thread_warning"))

        self.mainLayout.addWidget(self.threadNumSetting)
        self.mainLayout.addWidget(self.languageSetting)
        self.mainLayout.addWidget(self.nameLanguageSetting)
        self.mainLayout.addWidget(self.useGpuSetting)

        self.threadNumSetting.line.editingFinished.connect(self._warning_thread_count)
        self._warning_thread_count()

    def _warning_thread_count(self):
        max_threads = cpu_count()
        try:
            count = int(self.threadNumSetting.line.text())
        except ValueError:
            count = 32
            pass
        
        if count >= max_threads:
            self.threadNumSetting.label.setTextColor(QColor(255, 0, 0), QColor(255, 0,  0))
            self.warningLabel.setHidden(False)
        else:
            self.threadNumSetting.label.setTextColor(QColor(0, 0, 0), QColor(255, 255, 255))
            self.warningLabel.setHidden(True)
            
        
class GPUSettingFrame(BaseSettingFrame):
    def __init__(self, title: str | None = None, parent=None, root=None):
        super().__init__(title or tr("settings.gpu.title"), parent)
        self.root = root
        self.no_gpu_device_text = tr("settings.gpu.no_gpu_device")
        self.disabled_device_text = tr("settings.gpu.disabled_device")

        self.localSizeSetting = ComboBoxSettingItemFrame(
            ["32", "64", "128", "256", "512", "1024"],
            tr("settings.gpu.local_size"),
            cfg_key="local_size",
        )
        devices_info = get_device_info_c()
        if devices_info:
            self.gpuDeviceSetting = ComboBoxSettingItemFrame(
                devices_info,
                tr("settings.gpu.device"),
                cfg_key="device_name",
                type_input="str",
            )
        else:
            self.gpuDeviceSetting = ComboBoxSettingItemFrame(
                [self.no_gpu_device_text],
                tr("settings.gpu.device"),
                cfg_key="device_name",
                type_input="str",
            )
        self.gpuWorkerSetting = EditLineSettingItemFrame(
            tr("settings.gpu.max_gpu_workers"),
            None,
            cfg_obj=cfg.config,
            cfg_key="max_gpu_worker",
            min_value=0,
            max_value=128, 
            default_value=8,
            empty_invisible=False
        )
        self.gpuTestFrame = BaseSettingItemFrame("")
        self.gpu_test_button = PushButton(tr("settings.gpu.benchmark"))
        self.gpu_test_button.setMaximumWidth(160)
        self.gpuTestFrame.mainLayout.addWidget(self.gpu_test_button, Qt.AlignmentFlag.AlignRight)
        self.gpu_test_button.clicked.connect(self.__on_gpu_test_button_clicked)

        self.warningLabel = PixmapLabel()
        self.gpuDeviceSetting.labelLayout.addWidget(self.warningLabel)
        self.warningLabel.setPixmap(FluentIcon.INFO.icon().pixmap(16, 16))
        self.warningLabel.setFixedSize(16, 16)
        if devices_info:
            self.warningLabel.setToolTip(tr("settings.gpu.double_warning"))
        else:
            self.warningLabel.setToolTip(tr("settings.gpu.no_gpu_warning"))
        self.warningLabel.installEventFilter(ToolTipFilter(self.warningLabel, showDelay=0))

        if cfg.config.device_name == "cpu" and devices_info:
            self.gpuDeviceSetting.comboBox.setCurrentIndex(-1)
            self.gpuDeviceSetting.comboBox.setText(self.disabled_device_text)
        elif not devices_info:
            self.gpuDeviceSetting.comboBox.setCurrentIndex(-1)
            self.gpuDeviceSetting.comboBox.setText(self.no_gpu_device_text)
            self.gpuDeviceSetting.comboBox.setEnabled(False)
            cfg.config.device_name = "cpu"
            cfg.save()

        self._gpu_device_changed(self.gpuDeviceSetting.comboBox.currentIndex())
        set_gpu_max_worker_c(cfg.config.max_gpu_worker)

        self.gpuDeviceSetting.comboBox.currentIndexChanged.connect(self._gpu_device_changed)
        self.gpuWorkerSetting.line.editingFinished.connect(self._gpu_worker_changed)

        self.mainLayout.addWidget(self.localSizeSetting)
        self.mainLayout.addWidget(self.gpuDeviceSetting)
        self.mainLayout.addWidget(self.gpuWorkerSetting)
        self.mainLayout.addWidget(self.gpuTestFrame)

    def __on_gpu_test_button_clicked(self) -> None:
        GPUBenchmarkMessageBox(self.root).exec()

    def _gpu_worker_changed(self):
        self.gpuWorkerSetting.line._on_text_edited()
        set_gpu_max_worker_c(cfg.config.max_gpu_worker)

    def _gpu_device_changed(self, index):
        if self.gpuDeviceSetting.comboBox.text() == self.no_gpu_device_text:
            set_device_id_c(-1)
            self.warningLabel.setHidden(False)
            return
        set_device_id_c(index)
        self.warningLabel.setHidden(True)

    def resizeEvent(self, event: QResizeEvent) -> None:
        width = self.width()
        self.gpuDeviceSetting.comboBox.setMaximumWidth(width*2//3)
        self.gpuDeviceSetting.comboBox.setMinimumWidth(width*2//3)
        return super().resizeEvent(event)

class SettingInterface(SmoothScrollArea):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.expanded = True

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)

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

        if self.GPUSetting.gpuDeviceSetting.comboBox.text() == self.GPUSetting.no_gpu_device_text:
            self.normalSetting.useGpuSetting.switchButton.setChecked(0)
            self.normalSetting.useGpuSetting.switchButton.setEnabled(False)
            cfg.config.use_gpu = False
            cfg.save()

        self.GPUSetting.gpuDeviceSetting.comboBox.currentIndexChanged.connect(self._on_gpu_device_changed)
        self.normalSetting.useGpuSetting.switchButton.checkedChanged.connect(self._on_use_gpu_changed)

    def _on_gpu_device_changed(self, index):
        self.normalSetting.useGpuSetting.switchButton.setChecked(1)
    
    def _on_use_gpu_changed(self, checked):
        if not checked:
            self.GPUSetting.gpuDeviceSetting.comboBox.setCurrentIndex(-1)
            self.GPUSetting.gpuDeviceSetting.comboBox.setText(self.GPUSetting.disabled_device_text)
            self.GPUSetting.warningLabel.setHidden(True)
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

        self.GPUSetting = GPUSettingFrame(root=self)
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
