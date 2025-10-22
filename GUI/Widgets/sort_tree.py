from ..Compoents import SortTree
from ..Messenger import SortTreeMessages
from ..Compoents.Widgets.setting_window import SettingsWindow
from PySide6.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QApplication

from qfluentwidgets import VBoxLayout, PushButton, PopUpAniStackedWidget

from config import cfg

class SortTreeWidget(QWidget):


    def __init__(self, parent=None):
        super().__init__(parent)

        self.mainLayout = QVBoxLayout(self)

        self.stackedWidget = PopUpAniStackedWidget()

        self.stackedWidget.currentChanged.connect(self._on_stacked_widget_changed)



        self.mainLayout.addWidget(self.stackedWidget)

        self.tree = SortTree()

        self.addWidgetPopRight(self.tree)

        self.tree.setBorderRadius(8)
        self.tree.setBorderVisible(True)


        self.buttonLayout = QHBoxLayout()

        self.mainLayout.addLayout(self.buttonLayout)
        

        SortTreeMessages.CreateSettingsWindow.connect(self._create_settings_window)

    def addWidgetPopRight(self, widget: QWidget):
        self.stackedWidget.addWidget(widget, deltaX=76, deltaY=0)

    def _on_stacked_widget_changed(self, index: int):
        pass

    def _create_settings_window(self, obj):
        settings_window = SettingsWindow(context=obj)
        self.addWidgetPopRight(settings_window)
        self.stackedWidget.setCurrentWidget(settings_window)

    def add_item(self, texts):
        return self.tree.add_item(texts)

    def setBorderVisible(self, visible: bool):
        self.tree.setBorderVisible(visible)

    def setBorderRadius(self, radius: int):
        self.tree.setBorderRadius(radius)