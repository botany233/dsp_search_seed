__all__ = ["SortTree"]

from typing import Any
from PySide6.QtCore import QAbstractItemModel, Qt, QPoint, QSize
from PySide6.QtGui import QMouseEvent
from PySide6.QtWidgets import (
    QWidget,
    QTreeWidgetItem,
    QFileSystemModel,
    QHBoxLayout,
    QComboBox,
    QSizePolicy,
    QHeaderView,
    QGridLayout,
)

from qfluentwidgets import (
    TreeWidget,
    setTheme,
    Theme,
    TreeView,
    PushButton,
    VBoxLayout,
    ComboBox,
    RoundMenu,
    Action,
    LineEdit,
    BodyLabel,
    FluentLabelBase,
    FluentIcon,
    ToolButton,
    TransparentToolButton,
)

from .combox import AutoFixedComboBox

from ..Messenger import SortTreeMessages
from .. import star_types, planet_types, singularity, liquid

star_types = ["无 / 任意"] + star_types
planet_types = ["无 / 任意"] + planet_types
liquid = ["无 / 任意"] + liquid
singularity = ["无 / 任意"] + singularity

class TreeWidgetItem(QTreeWidgetItem):

    def __init__(self, *args, root: 'SortTree', **kwargs):
        self.root = root
        super().__init__(*args, **kwargs)
        self.setCheckState(0, Qt.CheckState.Checked)
        self.checkState(0)
        self.setSizeHint(0, QSize(0, 40))
        self.setFlags(self.flags() | Qt.ItemIsEditable)
        self._add_widgets_()

    def _add_widgets_(self):
        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.addButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

    def _on_add_button_clicked(self):
        if hasattr(self, 'addLeaf'):
            self.addLeaf()

    def _on_del_button_clicked(self):
        if self.parent() is None:
            self.root.takeTopLevelItem(self.root.indexOfTopLevelItem(self))
            return
        self.parent().removeChild(self)


class TreeWidgetLeave(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignRight)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.addButton = ToolButton(FluentIcon.ADD)
        self.addButton.setToolTip("点击添加子项")
        self.delButton = ToolButton(FluentIcon.DELETE)
        self.delButton.setToolTip("点击删除该项及其子项")

        self.mainLayout.addWidget(self.addButton)

        self.mainLayout.addSpacing(5)

        self.mainLayout.addWidget(self.delButton)

    def adjust_del_button(self):
        self.adjustButton = TransparentToolButton()
        self.mainLayout.insertWidget(0, self.adjustButton)
        pass
        
class RootTreeWidgetItem(TreeWidgetItem):
    """条件项"""
    def __init__(self, root: 'SortTree', *args, **kwargs):
        text = f"条件{root.topLevelItemCount() + 1}"
        texts = [text]
        super().__init__(root, texts, *args, root=root, **kwargs)

        self.__bind__widgets__()

    def __bind__widgets__(self):
       return
       # self.root.setItemWidget(self, 1, RootTreeLeave())
    

    def addLeaf(self) -> 'GalaxyTreeWidgetItem':
        if self.childCount() > 0:
            return  # type: ignore
        leaf = GalaxyTreeWidgetItem(self.root, self)
        self.addChild(leaf)
        self.setExpanded(True)
        return leaf

class GalaxyTreeWidgetItem(TreeWidgetItem):
    """银河系条件项"""
    def __init__(self, root: 'SortTree', branch: RootTreeWidgetItem,*args, **kwargs):
        text = "银河系条件"
        texts = [text]
        super().__init__(branch, texts, *args, root=root, **kwargs)
        self.branch = branch

        self.__bind__widgets__()
    
    def __bind__widgets__(self):
        self.leaf = SettingsTreeLeave()
        self.leaf.setText("矿物筛选 (银河系)")
        self.root.setItemWidget(self, 1, self.leaf)

    def addLeaf(self) -> 'SystemTreeWidgetItem':
        leaf = SystemTreeWidgetItem(self.root, self)
        self.addChild(leaf)
        self.setExpanded(True)
        return leaf

class SystemTreeWidgetItem(TreeWidgetItem):
    """恒星系条件项"""
    def __init__(self, root: 'SortTree', branch: GalaxyTreeWidgetItem,*args, **kwargs):
        text = f"恒星系条件{branch.childCount() + 1}"
        texts = [text]
        super().__init__(branch, texts, *args, root=root, **kwargs)
        self.branch = branch

        self.__bind__widgets__()
    
    def __bind__widgets__(self):
        self.root.setItemWidget(self, 1, SystemTreeLeave())
        pass

    def addLeaf(self) -> 'PlanetTreeWidgetItem':
        leaf = PlanetTreeWidgetItem(self.root, self)
        self.addChild(leaf)
        self.setExpanded(True)
        return leaf

# class StarTreeWidgetItem(TreeWidgetItem):
#     """恒星条件项"""
#     def __init__(self, root: 'SortTree', branch: SystemTreeWidgetItem,*args, **kwargs):
#         text = f"恒星条件{branch.childCount() + 1}"
#         texts = [text]
#         super().__init__(root, branch, texts, *args, **kwargs)
#         self.root = root
#         self.branch = branch

#         self.__bind__widgets__()
    
#     def __bind__widgets__(self):
#         pass

#     def addLeaf(self) -> 'PlanetTreeWidgetItem':
#         leaf = PlanetTreeWidgetItem(self.root, self)
#         self.addChild(leaf)
#         self.setExpanded(True)
#         return leaf

class PlanetTreeWidgetItem(TreeWidgetItem):
    """行星条件项"""
    def __init__(self, root: 'SortTree', branch: SystemTreeWidgetItem,*args, **kwargs):
        text = f"行星条件{branch.childCount() + 1}"
        texts = [text]
        super().__init__(branch, texts, *args, root=root, **kwargs)
        self.branch = branch
        self.manageButtons.addButton.setHidden(True)
        self.manageButtons.adjust_del_button()

        self.__bind__widgets__()
    
    def __bind__widgets__(self):
        self.root.setItemWidget(self, 1, PlanetTreeLeave())

    def addLeaf(self) -> None:
        return
        # leaf = MoonTreeWidgetItem(self.root, self)
        # self.addChild(leaf)
        # self.setExpanded(True)
        # return leaf

# class MoonTreeWidgetItem(TreeWidgetItem):
#     """卫星条件项"""
#     def __init__(self, root: 'SortTree', branch: PlanetTreeWidgetItem,*args, **kwargs):
#         text = f"卫星条件{branch.childCount() + 1}"
#         texts = [text]
#         super().__init__(root, branch, texts, *args, **kwargs)
#         self.root = root
#         self.branch = branch

#         self.__bind__widgets__()
    
#     def __bind__widgets__(self):
#         pass


class SettingsTreeLeave(PushButton):

    def _postInit(self):
        self.setText("矿物筛选")
        self.setIcon(FluentIcon.SETTING)
        self.clicked.connect(self._createSettingsWindow)
        self.root = self.parent()
        self.setFocusPolicy(Qt.NoFocus)

    def _createSettingsWindow(self):
        SortTreeMessages.CreateSettingsWindow.emit(self)

class SystemTreeLeave(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.starTypeLabel = BodyLabel("恒星类型")
        self.mainLayout.addWidget(self.starTypeLabel)
        self.starTypeComboBox = ComboBox()
        self.starTypeComboBox.addItems(star_types)
        self.mainLayout.addWidget(self.starTypeComboBox)
        self.luminosityLabel = BodyLabel("光度级别")
        self.mainLayout.addWidget(self.luminosityLabel)
        self.luminosityLineEdit = LineEdit()
        self.mainLayout.addWidget(self.luminosityLineEdit)
        self.luminosityLineEdit.setMaximumHeight(28)
        self.luminosityLineEdit.setFixedHeight(28)
        self.settingsButton = SettingsTreeLeave()
        self.settingsButton.setText("矿物筛选 (恒星系)")
        self.mainLayout.addWidget(self.settingsButton)
        self.settingsButton.setToolTip("设置恒星系筛选条件")

class PlanetTreeLeave(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignLeft)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.planetTypeLabel = BodyLabel("行星类型")
        self.mainLayout.addWidget(self.planetTypeLabel)
        self.planetTypeComboBox = AutoFixedComboBox()
        self.planetTypeComboBox.addItems(planet_types)
        self.mainLayout.addWidget(self.planetTypeComboBox)
        self.singularityLabel = BodyLabel("特点")
        self.mainLayout.addWidget(self.singularityLabel)
        self.singularityComboBox = AutoFixedComboBox()
        self.singularityComboBox.addItems(singularity)
        self.mainLayout.addWidget(self.singularityComboBox)
        self.liquidLabel = BodyLabel("液体类型")
        self.mainLayout.addWidget(self.liquidLabel)
        self.liquidComboBox = AutoFixedComboBox()
        self.liquidComboBox.addItems(liquid)
        self.mainLayout.addWidget(self.liquidComboBox)
        self.settingsButton = SettingsTreeLeave()
        self.mainLayout.addWidget(self.settingsButton)



class SortTree(TreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)


        self.setHeaderHidden(False)
        self.setEditTriggers(TreeWidget.NoEditTriggers)  # 先禁用自动编辑
        self.setUniformRowHeights(True)
        self.setColumnCount(3)

        self.setHeaderLabels(["名称", "条件", "管理"])

        header = self.header()
        header.setStretchLastSection(False)  # 最后一列拉伸
        header.setSectionResizeMode(0, QHeaderView.Interactive)
        header.setSectionResizeMode(1, QHeaderView.Stretch)
        header.setSectionResizeMode(2, QHeaderView.Fixed)

        self.setColumnWidth(0, 233)
        self.setColumnWidth(1, 422)
        self.setColumnWidth(2, 100)

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
            self.addLeaf()
        else:
            if hasattr(item, 'addLeaf'):
                item.addLeaf()
        
    def addLeaf(self) -> RootTreeWidgetItem:

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