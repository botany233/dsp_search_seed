__all__ = ["SortTree"]

from typing import Any
from PySide6.QtCore import Qt, QPoint, QSize
from PySide6.QtGui import QMouseEvent
from PySide6.QtWidgets import QTreeWidgetItem, QHeaderView
from qfluentwidgets import (
    TreeWidget, RoundMenu, Action, FluentIcon, TransparentToolButton,
    setCustomStyleSheet,
)

from .fields import (
    TreeWidgetLeave, GalaxyTreeLeave, StarTreeLeave, PlanetTreeLeave,
    ToolTipFilterWithEgg, moon_parent_planet_types, moon_parent_singularity,
    moon_planet_types, moon_singularity,
)
from ... import planet_types, singularity
from ...dsp_icons import AppIcons
from config import cfg
from config.cfg_dict_tying import GalaxyCondition, PlanetCondition, StarCondition
from language import tr, tr_domain


class TreeWidgetItem(QTreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj: Any, config_key: str, parent: Any = None):
        if parent is not None:
            super().__init__(parent)
        else:
            super().__init__()

        self.root = root
        self.config_obj = config_obj
        self.config_key = config_key

        self.setText(0, tr_domain("condition_names", self.config_obj.custom_name))
        self.setSizeHint(0, QSize(0, 40))
        self.setFlags(self.flags() | Qt.ItemIsEditable)
        self._update_check_state_from_config()
    
    # def add_widgets(self): ...

    def _update_check_state_from_config(self):
        if self.config_obj.checked:
            self.setCheckState(0, Qt.CheckState.Checked)
        else:
            self.setCheckState(0, Qt.CheckState.Unchecked)

    def setData(self, column: int, role: int, value: Any) -> None:
        if column == 0 and role == Qt.CheckStateRole:
            self.config_obj.checked = bool(value)
            cfg.save()
            super().setData(column, role, value)
            return

        if column == 0 and role == Qt.EditRole:
            self.config_obj.custom_name = value
            cfg.save()
        return super().setData(column, role, value)

    def GetIndex(self) -> int:
        if self.parent() is None:
            return self.root.indexOfTopLevelItem(self)
        return self.parent().indexOfChild(self)

    # def _on_add_button_clicked(self): ...

    def _on_del_button_clicked(self):
        if self.parent() is None:
            return

        belong_list = getattr(self.parent().config_obj, self.config_key)
        for i in range(len(belong_list)):
            if self.config_obj.uuid == belong_list[i].uuid:
                belong_list.pop(i)
                break
        else:
            raise Exception("uuid not found!")
        cfg.save()
        if isinstance(self, MoonTreeWidgetItem) and len(belong_list) == 0:
            self.parent()._switch_no_moons_planet()
        self.parent().removeChild(self)

class GalaxyTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj):
        super().__init__(root, config_obj, "galaxy_condition", root)

    def _on_add_planet_button_clicked(self):
        self.addPlanetLeaf()

    def _on_add_button_clicked(self):
        self.addStarLeaf()

    def add_widgets(self):
        self.leaf = GalaxyTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.addButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

        self.manageButtons.delButton.setHidden(True)
        # self.manageButtons.addButton.setHidden(False)
        self.manageButtons.addButton.setIcon(AppIcons.STAR)
        self.manageButtons.addPlanetButton.clicked.connect(self._on_add_planet_button_clicked)

    def addStarLeaf(self, new_star_condition: StarCondition|None = None) -> "StarTreeWidgetItem":
        if new_star_condition is None:
            new_star_condition = StarCondition()
            self.config_obj.star_condition.append(new_star_condition)
            cfg.save()

        index = 0
        while isinstance(self.child(index), StarTreeWidgetItem) and index < self.childCount():
            index += 1
        new_leaf = StarTreeWidgetItem(self.root, new_star_condition)
        self.insertChild(index, new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        return new_leaf

    def addPlanetLeaf(self, new_planet_condition: PlanetCondition|None = None) -> "PlanetTreeWidgetItem":
        if new_planet_condition is None:
            new_planet_condition = PlanetCondition()
            self.config_obj.planet_condition.append(new_planet_condition)
            cfg.save()

        new_leaf = PlanetTreeWidgetItem(self.root, new_planet_condition)
        self.addChild(new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        return new_leaf

class StarTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj: StarCondition):
        super().__init__(root, config_obj, "star_condition")

    def add_widgets(self):
        self.leaf = StarTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)
        self.leaf.load_config()

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.addButton.setHidden(True)
        self.manageButtons.addPlanetButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)
        self.manageButtons.addPlanetButton.setToolTip(tr("search.condition_tree.tooltips.add_planet"))

    def _on_add_button_clicked(self):
        self.addPlanetLeaf()

    def addPlanetLeaf(self, new_planet_condition: PlanetCondition|None = None) -> "PlanetTreeWidgetItem":
        if new_planet_condition is None:
            new_planet_condition = PlanetCondition()
            self.config_obj.planet_condition.append(new_planet_condition)
            cfg.save()

        new_leaf = PlanetTreeWidgetItem(self.root, new_planet_condition)
        self.addChild(new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        return new_leaf

class PlanetTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj: PlanetCondition):
        super().__init__(root, config_obj, "planet_condition")
        self.has_moon = False

    def add_widgets(self):
        self.leaf = PlanetTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)
        self.leaf.load_config()

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

        self.manageButtons.addButton.setHidden(True)
        self.manageButtons.addPlanetButton.setIcon(AppIcons.MOON)
        self.manageButtons.addPlanetButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.addPlanetButton.setToolTip(tr("search.condition_tree.tooltips.add_moon"))

    def _on_add_button_clicked(self):
        self.addMoonLeaf()
    
    def addMoonLeaf(self, new_planet_condition: PlanetCondition|None = None) -> "MoonTreeWidgetItem":
        if new_planet_condition is None:
            new_planet_condition = PlanetCondition()
            new_planet_condition.custom_name = "卫星条件"
            self.config_obj.moon_conditions.append(new_planet_condition)
            cfg.save()

        new_leaf = MoonTreeWidgetItem(self.root, new_planet_condition)
        self.addChild(new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        self._switch_moons_planet()
        return new_leaf

    def _switch_moons_planet(self):
        if self.has_moon:
            return
        self.leaf.planetTypeComboBox.clear()
        self.leaf.planetTypeComboBox.addItems(moon_parent_planet_types)
        self.leaf.planetTypeComboBox.load_config()
        self.leaf.singularityComboBox.clear()
        self.leaf.singularityComboBox.addItems(moon_parent_singularity)
        self.leaf.singularityComboBox.load_config()

        self.has_moon = True

    def _switch_no_moons_planet(self):
        if not self.has_moon:
            return
        self.leaf.planetTypeComboBox.clear()
        self.leaf.planetTypeComboBox.addItems(planet_types)
        self.leaf.planetTypeComboBox.load_config()
        self.leaf.singularityComboBox.clear()
        self.leaf.singularityComboBox.addItems(singularity)
        self.leaf.singularityComboBox.load_config()

        self.has_moon = False

class MoonTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj: PlanetCondition):
        super().__init__(root, config_obj, "moon_conditions")
    
    def add_widgets(self):
        self.leaf = PlanetTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)
        self.leaf.load_config()

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

        self.manageButtons.addButton.setHidden(True)
        self.manageButtons.addPlanetButton.setHidden(True)
        self.manageButtons.adjustButton = TransparentToolButton()
        self.manageButtons.adjustButton.setToolTip(tr("search.condition_tree.egg.init"))
        self.manageButtons.adjustButton.installEventFilter(ToolTipFilterWithEgg(self.manageButtons.adjustButton, showDelay=5000))
        qss = """TransparentToolButton:hover{background: transparent}"""
        setCustomStyleSheet(self.manageButtons.adjustButton, qss, qss)
        self.manageButtons.mainLayout.insertWidget(0, self.manageButtons.adjustButton)

        self._moon_diff()

    def _moon_diff(self):
        self.leaf.singularityComboBox.clear()
        self.leaf.singularityComboBox.addItems(moon_singularity)
        self.leaf.singularityComboBox.load_config()
        self.leaf.planetTypeComboBox.clear()
        self.leaf.planetTypeComboBox.addItems(moon_planet_types)
        self.leaf.planetTypeComboBox.load_config()

class SortTree(TreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.leaf = None
        self.setIndentation(15)
        # self.setItemsExpandable(False)
        # self.setExpandsOnDoubleClick(False)
        self.setHeaderHidden(False)
        self.setEditTriggers(TreeWidget.NoEditTriggers)  # 先禁用自动编辑
        self.setUniformRowHeights(True)
        self.setColumnCount(3)

        self.setHeaderLabels([tr("search.condition_tree.headers.name"), tr("search.condition_tree.headers.condition"), tr("search.condition_tree.headers.manage")])

        header = self.header()
        header.setStretchLastSection(False)  # 最后一列拉伸
        header.setSectionResizeMode(0, QHeaderView.Interactive)
        header.setSectionResizeMode(1, QHeaderView.Stretch)
        header.setSectionResizeMode(2, QHeaderView.Fixed)

        self.setColumnWidth(0, 201)
        self.setColumnWidth(1, 575)
        self.setColumnWidth(2, 90)

        self.root = self.invisibleRootItem()

        self.root.setExpanded(True)

        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self.on_custom_context_menu_requested)
    #     self.itemCollapsed.connect(lambda item: item.setExpanded(True))

    # def drawBranches(self, painter, rect, index):
    #     return

    def on_custom_context_menu_requested(self, pos: QPoint):
        item = self.itemAt(pos)
        if item is None:
            return
        menu = RoundMenu("test", self)
        if not isinstance(item, GalaxyTreeWidgetItem):
            del_action = Action(tr("search.condition_tree.menu.delete"))
            del_action.triggered.connect(
                lambda: self.on_menu_del_action_triggered(item)
            )
            menu.addAction(del_action)

        if not isinstance(item, (PlanetTreeWidgetItem, MoonTreeWidgetItem)):
            add_action = Action(tr("search.condition_tree.menu.add_planet"))
            add_action.triggered.connect(
                lambda: self.on_menu_add_action_triggered(item)
            )
            menu.addAction(add_action)
        if isinstance(item, GalaxyTreeWidgetItem):
            add_star_action = Action(tr("search.condition_tree.menu.add_star"))
            add_star_action.triggered.connect(
                lambda: self.on_menu_add_star_action_triggered(item)
            )
            menu.addAction(add_star_action)

        if isinstance(item, PlanetTreeWidgetItem):
            add_moon_action = Action(tr("search.condition_tree.menu.add_moon"))
            add_moon_action.triggered.connect(
                lambda: self.on_menu_add_moon_action_triggered(item)
            )
            menu.addAction(add_moon_action)

        pos = QPoint(pos.x() + 10, pos.y() + 30)
        menu.exec(self.mapToGlobal(pos))
        menu.closedSignal.connect(menu.deleteLater)

    def on_menu_del_action_triggered(self, item: TreeWidgetItem):
        item._on_del_button_clicked()

    def on_menu_add_moon_action_triggered(self, item: PlanetTreeWidgetItem):
        item.addMoonLeaf()

    def on_menu_add_star_action_triggered(self, item: GalaxyTreeWidgetItem):
        item.addStarLeaf()

    def on_menu_add_action_triggered(self, item: GalaxyTreeWidgetItem|StarTreeWidgetItem):
        item.addPlanetLeaf()

    def addLeaf(self, new_galaxy_condition: GalaxyCondition) -> GalaxyTreeWidgetItem:
        self.leaf = GalaxyTreeWidgetItem(self, new_galaxy_condition)
        self.leaf.add_widgets()
        return self.leaf

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
