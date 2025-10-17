from PySide6.QtCore import QAbstractItemModel, Qt, QPoint, QSize
from PySide6.QtGui import QMouseEvent
from PySide6.QtWidgets import QWidget, QTreeWidgetItem, QFileSystemModel, QHBoxLayout, QComboBox, QSizePolicy

from qfluentwidgets import TreeWidget, setTheme, Theme, TreeView, PushButton, VBoxLayout, ComboBox, RoundMenu, Action, LineEdit, BodyLabel, FluentLabelBase, FluentIcon

from .Widgets.setting_window import SettingMessageBox
from .Widgets.labels import AlwaysShowLabel

__all__ = ["SortTree"]


class TreeWidgetItem(QTreeWidgetItem):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setCheckState(0, Qt.CheckState.Checked)
        self.checkState(0)
        self.setSizeHint(0, QSize(0, 40))
        self.setFlags(self.flags() | Qt.ItemIsEditable)


    def add_child(self, texts: list[str] | str) -> 'TreeWidgetItem | None':
        if isinstance(texts, list):
            if len(texts) == 0:
                return None
            text = [texts[0]]
            child = TreeWidgetItem(text)
            child.add_child(texts[1:])
            self.addChild(child)
            return child
        else:
            child = TreeWidgetItem(texts)
            self.addChild(child)
            return child

class RootTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: 'SortTree', *args, **kwargs):
        text = f"条件{root.topLevelItemCount() + 1}"
        texts = [text]
        super().__init__(root, texts, *args, **kwargs)

        self.root = root

        self.__bind__widgets__()

    def __bind__widgets__(self):
        self.root.setItemWidget(self, 1, RootTreeLeave())

class RootTreeLeave(PushButton):

    WindowCreated = False


    def _postInit(self):
        self.setText("种子矿物筛选")
        self.setIcon(FluentIcon.SETTING)
        self.clicked.connect(self._createSettingsWindow)
        self.root = self.parent()

    def _createSettingsWindow(self):
        self.WindowCreated = not self.WindowCreated
        if self.WindowCreated:
            return
        box = SettingMessageBox(self.parent())
        box.show()
        pass


class SortTree(TreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.setHeaderHidden(False)
        self.setEditTriggers(TreeWidget.NoEditTriggers)  # 先禁用自动编辑
        self.setUniformRowHeights(True)
        self.setColumnCount(2)

        self.setHeaderLabels(["名称", "条件"])

        self.setColumnWidth(0, 200)

        self.root = self.invisibleRootItem()

        self.root.setExpanded(True)

        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self.on_custom_context_menu_requested)

    def on_custom_context_menu_requested(self, pos: QPoint):
        item = self.itemAt(pos)
        menu = RoundMenu("test", self)
        if item is not None:
            del_action = Action("删除项")
            del_action.triggered.connect(lambda: self.on_menu_del_action_triggered(item))
            menu.addAction(del_action)

        add_action = Action("添加子项")
        add_action.triggered.connect(lambda: self.on_menu_add_action_triggered(item))
        menu.addAction(add_action)
        pos = QPoint(pos.x() + 10, pos.y() + 30)
        menu.exec(self.mapToGlobal(pos))


    def on_menu_del_action_triggered(self, item: QTreeWidgetItem):
        if item.parent() is None:
            self.takeTopLevelItem(self.indexOfTopLevelItem(item))
            return
        item.parent().removeChild(item)


    def on_menu_add_action_triggered(self, item: QTreeWidgetItem):
        if item is None:
            self.add_items()
        else:
            pass
        pass
    def add_item(self, texts: list[str] | str) -> TreeWidgetItem | None:
        if isinstance(texts, list):
            if len(texts) == 0:
                return None
            text = [texts[0]]
            item = TreeWidgetItem(text)
            self.setItemWidget(item, 1, LineEdit())
            item.add_child(texts[1:])
            self.addTopLevelItem(item)
            return item
        else:
            item = TreeWidgetItem(texts)
            self.addTopLevelItem(item)
            return item
        
    def add_items(self) -> RootTreeWidgetItem:

        leaf = RootTreeWidgetItem(self)
        return leaf

    def mouseDoubleClickEvent(self, event: QMouseEvent):
        # 获取点击位置的项目
        item = self.itemAt(event.position().toPoint())
        if item:
            text = item.text(0)

            label_length = self.fontMetrics().horizontalAdvance(text)
            
            item_rect = self.visualItemRect(item)
            text_x_start = item_rect.x() + 40
            text_x_end = text_x_start + label_length + 10
            if text_x_start <= event.position().x() <= text_x_end:
                # 如果点击在文本区域，允许编辑
                self.setEditTriggers(TreeWidget.DoubleClicked)
                super().mouseDoubleClickEvent(event)
                self.setEditTriggers(TreeWidget.NoEditTriggers)  # 编辑后禁用自动编辑
                return
            
        # 如果不是点击文本区域，执行默认行为
        super().mouseDoubleClickEvent(event)