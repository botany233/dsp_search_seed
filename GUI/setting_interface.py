from PySide6.QtGui import QEnterEvent, QMouseEvent, QResizeEvent, QColor
from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QGridLayout
from PySide6.QtCore import Qt
from qfluentwidgets import (
    TitleLabel,
    BodyLabel,
    isDarkTheme,
    SmoothScrollArea,
    ToolButton,
    TransparentToolButton,
    FluentIcon,
    setCustomStyleSheet,
    MessageBoxBase,
    LineEdit,
    getFont,
)
from multiprocessing import cpu_count
from typing import Literal
from config import cfg
from logger import log

from .Compoents import LimitLineEdit

class SettingInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.mainLayout = QHBoxLayout(self)
        self.__init_left()
        self.__init_right()
        # self.mainLayout.setStretch(0, 1)
        # self.mainLayout.setStretch(1, 2)

    def __init_left(self):
        self.leftWidget = QWidget()
        # self.leftWidget.setFixedWidth(263)
        self.mainLayout.addWidget(self.leftWidget)
        self.leftLayout = QGridLayout(self.leftWidget)
        self.leftLayout.setContentsMargins(0, 0, 0, 0)

        self.leftLayout.addWidget(TitleLabel("基础设置"))
        widgets = [
            BodyLabel("最大线程数:"),
            LimitLineEdit("max_thread", min_value=1, max_value=128, default_value=cpu_count(), empty_invisible=False)
        ]
        for index, widget in enumerate(widgets):
            self.leftLayout.addWidget(widget, index//2+1, index%2)

    def __init_right(self):
        self.rightWidget = QWidget()
        self.mainLayout.addWidget(self.rightWidget)
        self.rightLayout = QGridLayout(self.rightWidget)
        self.rightLayout.setContentsMargins(0, 0, 0, 0)

        self.rightLayout.addWidget(TitleLabel("GPU设置"))
        widgets = [
            BodyLabel("工作组大小："),
            LimitLineEdit("local_size", min_value=32, max_value=1024, default_value=256, empty_invisible=False),
            BodyLabel("设备id："),
            LimitLineEdit("device_id", min_value=-1, max_value=1024, default_value=-1, empty_invisible=False),
        ]
        for index, widget in enumerate(widgets):
            self.rightLayout.addWidget(widget, index//2+1, index%2)

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

#         self.mainLayout = QHBoxLayout(self)
#         self.mainLayout.setContentsMargins(15, 0, 15, 0)
#         self.label = BodyLabel(title)
#         self.mainLayout.addWidget(self.label)

#         self.setMinimumHeight(40)

#     def _cfg_save(self, value):
#         try:
#             if self.cfg_obj and self.cfg_key:
#                 setattr(self.cfg_obj, self.cfg_key, value)
#                 cfg.save()
#         except Exception as e:
#             log.error(f"设置配置项 {self.cfg_obj} 失败: {e}")
    
#     def _cfg_load(self):
#         try:
#             if self.cfg_obj and self.cfg_key:
#                 return getattr(self.cfg_obj, self.cfg_key)
#         except Exception as e:
#             log.error(f"获取配置项 {self.cfg_obj} 失败: {e}")
#         return None

# class ButtonSettingItemFrame(BaseSettingItemFrame):
#     def __init__(self, title: str = "Setting Item", parent=None):
#         super().__init__(title, parent)

#         self.isPressed = False

#         self.button = TransparentToolButton()
#         def passPressEvent(e):
#             self.parent().mousePressEvent(e)
#             return
#         def passReleaseEvent(e):
#             self.parent().mouseReleaseEvent(e)
#             return
#         self.button.mousePressEvent = passPressEvent
#         self.button.mouseReleaseEvent = passReleaseEvent
#         qss = """
#         TransparentToolButton:hover{
#             border: none;
#             background: transparent;
#             }
#         """
#         if isDarkTheme():
#             item_qss = """
#                 ButtonSettingItemFrame:hover{
#                     background-color: rgba(255, 255, 255, 9);
#                 }
#             """
#         else:
#             item_qss = """
#                 ButtonSettingItemFrame:hover{
#                     background-color: rgba(0, 0, 0, 9);
#                 }
#             """
#         self.setStyleSheet(item_qss)
#         setCustomStyleSheet(self.button, qss, qss)
#         self.mainLayout.addWidget(self.button)
#         self.button.setIcon(FluentIcon.CHEVRON_RIGHT)

#     def enterEvent(self, event: QEnterEvent) -> None:
#         self.setCursor(Qt.CursorShape.PointingHandCursor)
#         return super().enterEvent(event)

#     def mousePressEvent(self, event: QMouseEvent) -> None:
#         self.button.isPressed = True
#         self.label.setTextColor(dark=QColor(255,255,255,150), light=QColor(0,0,0,150))
#         return super().mousePressEvent(event)

#     def mouseReleaseEvent(self, event: QMouseEvent) -> None:
#         self.button.isPressed = False
#         self.label.setTextColor(dark=QColor(255,255,255), light=QColor(0,0,0))
#         hit_widget = self.childAt(event.position().toPoint())
#         if hit_widget:
#             self.button.click()
#         return super().mouseReleaseEvent(event)

# class EditLineSettingItemFrame(BaseSettingItemFrame):
#     def __init__(
#         self,
#         title: str = "Setting Item",
#         parent=None,
#         cfg_obj=None,
#         cfg_key: str = "",
#         type_input: Literal["int", "float", "str"] = "int",
#         min_value: int|float|None = None,
#         max_value: int|float|None = None,
#         default_value: int|float|str = "",
#         empty_invisible: bool = True,
#     ):
#         super().__init__(title, parent, cfg_obj, cfg_key)

#         self.line = LimitLineEdit(cfg_key, cfg_obj, type_input=type_input, empty_invisible=empty_invisible, min_value=min_value, max_value=max_value, default_value=default_value)
#         self.mainLayout.addWidget(self.line)
#         self.mainLayout.setAlignment(self.line, Qt.AlignmentFlag.AlignRight)

# class IEditLine(EditLineSettingItemFrame):
#     def __init__(self, title: str = "Setting Item", parent=None, cfg_obj=None, cfg_key: str = "", type_input: Literal['int'] | Literal['float'] | Literal['str'] = "int", min_value: int | float | None = None, max_value: int | float | None = None, default_value: int | float | str = "", empty_invisible: bool = True):
#         super().__init__(title, parent, cfg_obj, cfg_key, type_input, min_value, max_value, default_value, empty_invisible)
#         self.mainLayout.setContentsMargins(0,0,0,0)

# class CPUSearchSettingMessageBox(BaseBox):
#     def __init__(self, title: str = "搜索设置", parent=None):
#         super().__init__(title, parent)

#         self.batchSizeLine = IEditLine("批处理大小", cfg_obj=cfg.config, cfg_key="batch_size", min_value=1, max_value=4096, default_value=64, empty_invisible=False)
#         self.label_thread_num = BodyLabel("进程数:")
#         self.batchSizeLine.line.setPlaceholderText("256")
#         self.mainLayout.addWidget(self.batchSizeLine)

# class CPUSettingFrame(BaseSettingFrame):
#     def __init__(self, title: str = "CPU 设置", parent=None):
#         super().__init__(title, parent)

#         self.searchSettingI = ButtonSettingItemFrame("搜索设置")
#         self.mainLayout.addWidget(self.searchSettingI)

#         self.searchSettingI.button.clicked.connect(self._on_search_setting_clicked)

#     def _on_search_setting_clicked(self):
#         msgBox = CPUSearchSettingMessageBox(
#             parent=self.searchSettingI.window())
#         if msgBox.exec():
#             pass

# class GPUSettingFrame(BaseSettingFrame):
#     def __init__(self, title: str = "GPU 设置", parent=None):
#         super().__init__(title, parent)

#         self.item1 = ButtonSettingItemFrame("启用 GPU 加速")
#         self.mainLayout.addWidget(self.item1)

# class SettingInterface(SmoothScrollArea):
#     def __init__(self, parent=None):
#         super().__init__(parent)

#         self.expanded = True

#         self.scrollWidget = QFrame()
#         self.setWidget(self.scrollWidget)
#         self.setWidgetResizable(True)
#         self.mainLayout = QHBoxLayout(self.scrollWidget)
#         self.mainLayout.setContentsMargins(10, 0, 10, 0)

#         self._init_left_layout()
#         self._init_right_layout()

#         self.mainLayout.addLayout(self.leftLayout)
#         self.mainLayout.addLayout(self.rightLayout)

#         self.enableTransparentBackground()

#     def _init_left_layout(self):
#         self.leftLayout = QVBoxLayout()
#         self.leftLayout.setContentsMargins(10, 10, 10, 10)

#         self.leftLayout.label = CPUSettingFrame()
#         self.leftLayout.addWidget(self.leftLayout.label)

#         self.leftLayout.addStretch()

#     def _init_right_layout(self):
#         self.rightLayout = QVBoxLayout()
#         self.rightLayout.setContentsMargins(10, 10, 10, 10)

#         self.rightLayout.label = BaseSettingFrame("高级设置")
#         self.rightLayout.addWidget(self.rightLayout.label)


#         self.rightLayout.addStretch()

#     def __bind_right_to_left(self):
#         if not self.expanded:
#             return
#         self.expanded = False
#         self.rightLayout.setContentsMargins(0, 0, 0, 0)
#         self.rightLayout.setParent(None)
#         stretch = self.leftLayout.itemAt(self.leftLayout.count() - 1)
#         if stretch and stretch.spacerItem():
#             self.leftLayout.removeItem(stretch)

#         self.leftLayout.addLayout(self.rightLayout)
#         pass

#     def __rebind_right(self):
#         if self.expanded:
#             return
#         self.expanded = True
#         self.rightLayout.setContentsMargins(10, 10, 10, 10)
#         self.rightLayout.setParent(None)
#         self.mainLayout.addLayout(self.rightLayout)
#         self.leftLayout.addStretch()
#         pass

#     def resizeEvent(self, event: QResizeEvent) -> None:
#         width = self.width()
#         if width < 1000:
#             self.__bind_right_to_left()
#         elif not self.expanded:
#             self.__rebind_right()

#         return super().resizeEvent(event)