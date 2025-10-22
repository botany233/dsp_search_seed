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
        
        self.clearButton = PushButton("清空")
        self.clearButton.clicked.connect(self.__on_clear_button_clicked__)
        self.addItemButton = PushButton("添加项")
        self.buttonLayout.addWidget(self.clearButton)
        self.buttonLayout.addWidget(self.addItemButton)

        self.addItemButton.clicked.connect(self._on_add_item_button_clicked__)

        SortTreeMessages.CreateSettingsWindow.connect(self._create_settings_window)

    def addWidgetPopRight(self, widget: QWidget):
        self.stackedWidget.addWidget(widget, deltaX=76, deltaY=0)

    def _on_stacked_widget_changed(self, index: int):
        if hasattr(self, 'buttonLayout'):
            if index != 0:
                self.clearButton.setHidden(True)
                self.addItemButton.setHidden(True)
            else:
                self.clearButton.setHidden(False)
                self.addItemButton.setHidden(False)
        pass

    def _on_add_item_button_clicked__(self):
        item = self.tree.currentItem()
        if item:
            if hasattr(item, 'addLeaf'):
                leaf = item.addLeaf()
                while leaf is not None:
                    leaf = leaf.addLeaf()
        else:
            leaf = self.tree.addLeaf()
            while leaf is not None:
                leaf = leaf.addLeaf()

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

    def __on_clear_button_clicked__(self):
        self.tree.clear()
        cfg.config.conditions = []
        cfg.save()
