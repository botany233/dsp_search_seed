__all__ = ["SortTree"]

from typing import Any
from PySide6.QtCore import QAbstractItemModel, Qt, QPoint, QSize, Signal
from PySide6.QtGui import QMouseEvent, QIcon
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
    SwitchButton,
    setFont,
)

from .combox import AutoFixedComboBox
from .Widgets.line_edit import LimitLineEdit
from .Widgets.button import ConfigSwitchButton


from config import cfg
from config.cfg_dict_tying import (
    GalaxyCondition,
    PlanetCondition,
    StarCondition,
    VeinsName,
)
from ..Messenger import SortTreeMessages
from .. import star_types, planet_types, singularity, liquid, dsp_level
from GUI.dsp_icons import AppIcons


star_types = ["无限制"] + star_types
planet_types = ["无限制"] + planet_types
liquid = ["无限制"] + liquid
singularity = ["无限制"] + singularity
dsp_level = ["无限制"] + dsp_level


class LeaveBase(QWidget):
    def __init__(self, parent=None, config_obj=None):
        self.config_obj = config_obj
        super().__init__(parent)
        qss = """QLabel{font-size:12px;}"""
        self.setStyleSheet(qss)
        setFont(self, 12)


    def load_config(self):
        pass

class TreeWidgetItem(QTreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj: Any, config_key: str, parent: Any = None):
        if parent is not None:
            super().__init__(parent)
        else:
            super().__init__()

        self.root = root
        self.config_obj = config_obj
        self.config_key = config_key

        self.setText(0, self.config_obj.custom_name)
        self.setSizeHint(0, QSize(0, 40))
        self.setFlags(self.flags() | Qt.ItemIsEditable)
        self._update_check_state_from_config()
    
    def add_widgets(self): ...

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

    def _on_add_button_clicked(self): ...

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
        self.parent().removeChild(self)

class TreeWidgetLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignRight)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.addButton = ToolButton(AppIcons.STAR)
        self.addButton.setToolTip("添加恒星条件")
        self.delButton = ToolButton(FluentIcon.DELETE)
        self.delButton.setToolTip("点击删除该项及其子项")
        self.addPlanetButton = ToolButton(AppIcons.PLANET)
        self.addPlanetButton.setToolTip("添加星球条件")

        self.mainLayout.addWidget(self.addButton)

        self.mainLayout.addSpacing(5)

        self.mainLayout.addWidget(self.delButton)

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
        self.manageButtons.addButton.setHidden(False)
        self.manageButtons.addButton.setIcon(AppIcons.STAR)
        self.manageButtons.mainLayout.addWidget(self.manageButtons.addPlanetButton)
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
        self.manageButtons.addButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)
        self.manageButtons.addButton.setToolTip("添加星球条件")

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

    def add_widgets(self):
        self.leaf = PlanetTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)
        self.leaf.load_config()

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.addButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

        self.manageButtons.addButton.setHidden(True)
        self.manageButtons.adjustButton = TransparentToolButton()
        self.manageButtons.mainLayout.insertWidget(0, self.manageButtons.adjustButton)

    def _on_add_button_clicked(self):
        return

class SettingsTreeLeave(QWidget):
    def __init__(
        self,
        parent=None,
        buttonText: str = "矿物",
        items: dict[str, str] | None = None,
        config_obj=None,
        config_key: str | None = None,
        obj_name: str = "",
        description: str | None = None,
    ):
        """
        Args:
            items (dict[str, str]):
                键: 设置名称
                值: 显示名称
        """
        super().__init__(parent)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.settingsButton = PushButton()
        self.settingsButton.setIcon(FluentIcon.SETTING)
        self.settingsButton.setText(buttonText)
        self.settingsButton.clicked.connect(self._createSettingsWindow)
        self.mainLayout.addWidget(self.settingsButton)
        self.items = items
        self.description = description
        if obj_name != "":
            self.settingsButton.setObjectName(obj_name)
        else:
            self.settingsButton.setObjectName(buttonText)
        # self.setFocusPolicy(Qt.NoFocus)
        self.config_obj = config_obj
        self.config_key = config_key

    def _createSettingsWindow(self):
        SortTreeMessages.CreateSettingsWindow.emit(self)


class GalaxyTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.veinsConditionButton = SettingsTreeLeave(
            buttonText="矿簇",
            config_obj=config_obj,
            config_key="veins_condition",
            items=VeinsName().model_dump(),
            obj_name="星系矿簇筛选",
            description="设置星系内最少有多少对应矿簇, 不填写则不限制",
        )
        self.veinsPointConditionButton = SettingsTreeLeave(
            buttonText="矿脉",
            config_obj=config_obj,
            config_key="veins_point_condition",
            items=VeinsName().model_dump(),
            obj_name="星系矿脉筛选",
            description="设置星系内最少有多少对应矿脉, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.veinsConditionButton)
        self.mainLayout.addWidget(self.veinsPointConditionButton)


class StarTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.starTypeLabel = BodyLabel("恒星类型")
        self.mainLayout.addWidget(self.starTypeLabel)
        self.starTypeComboBox = AutoFixedComboBox(config_key="star_type")
        self.starTypeComboBox.addItems(star_types)
        self.mainLayout.addWidget(self.starTypeComboBox)
        self.luminosityLabel = BodyLabel("光度级别")
        self.mainLayout.addWidget(self.luminosityLabel)
        self.luminosityLineEdit = LimitLineEdit("lumino_level", self.config_obj, "float", 0, default_value=0.0)
        self.mainLayout.addWidget(self.luminosityLineEdit)

        self.distanceLabel = BodyLabel("最远距离")
        self.mainLayout.addWidget(self.distanceLabel)
        self.distanceLineEdit = LimitLineEdit("distance_level", self.config_obj, "float", 0, default_value=-1.0)
        self.mainLayout.addWidget(self.distanceLineEdit)

        self.hitStarNumLabel = BodyLabel("符合数量")
        self.mainLayout.addWidget(self.hitStarNumLabel)
        self.hitStarNumLineEdit = LimitLineEdit("satisfy_num", self.config_obj, min_value = 1, default_value=1, empty_invisible=False)
        self.mainLayout.addWidget(self.hitStarNumLineEdit)
        self.distanceLineEdit.setMaximumHeight(28)
        self.distanceLineEdit.setFixedHeight(28)
        self.hitStarNumLineEdit.setMaximumHeight(28)
        self.hitStarNumLineEdit.setFixedHeight(28)
        self.luminosityLineEdit.setMaximumHeight(28)
        self.luminosityLineEdit.setFixedHeight(28)
        self.veinsConditionButton = SettingsTreeLeave(
            buttonText="矿簇",
            config_obj=config_obj,
            config_key="veins_condition",
            items=VeinsName().model_dump(),
            obj_name="恒星系矿簇筛选",
            description="设置恒星系内最少有多少对应矿簇, 不填写则不限制",
        )
        self.veinsPointConditionButton = SettingsTreeLeave(
            buttonText="矿脉",
            config_obj=config_obj,
            config_key="veins_point_condition",
            items=VeinsName().model_dump(),
            obj_name="恒星系矿脉筛选",
            description="设置恒星系内最少有多少对应矿脉, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.veinsConditionButton)
        # self.veinsConditionButton.setToolTip("设置恒星系矿簇条件")
        self.mainLayout.addWidget(self.veinsPointConditionButton)
        # self.veinsPointConditionButton.setToolTip("设置恒星系矿脉条件")

    def load_config(self):
        self.starTypeComboBox.load_config()


class PlanetTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignLeft)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.planetTypeLabel = BodyLabel("星球类型")
        self.mainLayout.addWidget(self.planetTypeLabel)
        self.planetTypeComboBox = AutoFixedComboBox(config_key="planet_type")
        self.planetTypeComboBox.addItems(planet_types)
        self.mainLayout.addWidget(self.planetTypeComboBox)
        self.singularityLabel = BodyLabel("特点")
        self.mainLayout.addWidget(self.singularityLabel)
        self.singularityComboBox = AutoFixedComboBox(config_key="singularity")
        self.singularityComboBox.addItems(singularity)
        self.mainLayout.addWidget(self.singularityComboBox)
        self.liquidLabel = BodyLabel("液体")
        self.mainLayout.addWidget(self.liquidLabel)
        self.liquidComboBox = AutoFixedComboBox(config_key="liquid_type")
        self.liquidComboBox.addItems(liquid)
        self.mainLayout.addWidget(self.liquidComboBox)
        self.dspLevelLabel = BodyLabel("戴森球接收")
        self.mainLayout.addWidget(self.dspLevelLabel)
        self.dspLevelComboBox = AutoFixedComboBox(config_key="dsp_level")
        self.dspLevelComboBox.addItems(dsp_level)
        self.mainLayout.addWidget(self.dspLevelComboBox)
        # self.fullCoverdPlanetSwitch = ConfigSwitchButton("是<u><i>否</i></u>全包", indicatorPos=1)
        # self.fullCoverdPlanetSwitch.setOnText("<u><i>是</i></u>否全包")
        # self.mainLayout.addWidget(self.fullCoverdPlanetSwitch)
        # self.fullCoverdPlanetSwitch.set_config(config_obj=config_obj, config_key="is_in_dsp")
        # self.fullReceivePlanetSwitch = ConfigSwitchButton("是<u><i>否</i></u>全接收", indicatorPos=1)
        # self.fullReceivePlanetSwitch.setOnText("<u><i>是</i></u>否全接收")
        # self.mainLayout.addWidget(self.fullReceivePlanetSwitch)
        # self.fullReceivePlanetSwitch.set_config(config_obj=config_obj, config_key="is_on_dsp")
        self.hitStarNumLabel = BodyLabel("符合数量")
        self.mainLayout.addWidget(self.hitStarNumLabel)
        self.hitStarNumLineEdit = LimitLineEdit("satisfy_num", self.config_obj, min_value = 1, default_value=1, empty_invisible=False)
        self.mainLayout.addWidget(self.hitStarNumLineEdit)

        self.hitStarNumLineEdit.setMaximumHeight(28)
        self.hitStarNumLineEdit.setFixedHeight(28)

        self.veinsConditionButton = SettingsTreeLeave(
            buttonText="矿簇",
            config_obj=config_obj,
            config_key="veins_condition",
            items=VeinsName().model_dump(),
            obj_name="星球矿簇筛选",
            description="设置星球内最少有多少对应矿簇, 不填写则不限制",
        )
        self.veinsPointConditionButton = SettingsTreeLeave(
            buttonText="矿脉",
            config_obj=config_obj,
            config_key="veins_point_condition",
            items=VeinsName().model_dump(),
            obj_name="星球矿脉筛选",
            description="设置星球内最少有多少对应矿脉, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.veinsConditionButton)
        self.mainLayout.addWidget(self.veinsPointConditionButton)

    def load_config(self):
        self.planetTypeComboBox.load_config()
        self.singularityComboBox.load_config()
        self.liquidComboBox.load_config()


class SortTree(TreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.leaf = None
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

        self.setColumnWidth(0, 170)
        self.setColumnWidth(1, 550)
        self.setColumnWidth(2, 100)

        self.root = self.invisibleRootItem()
        
        self.root.setExpanded(True)

        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self.on_custom_context_menu_requested)

    def on_custom_context_menu_requested(self, pos: QPoint):
        item = self.itemAt(pos)
        if item is None:
            return
        menu = RoundMenu("test", self)
        if not isinstance(item, GalaxyTreeWidgetItem):
            del_action = Action("删除项")
            del_action.triggered.connect(
                lambda: self.on_menu_del_action_triggered(item)
            )
            menu.addAction(del_action)

        if not isinstance(item, PlanetTreeWidgetItem):
            add_action = Action("添加星球条件")
            add_action.triggered.connect(
                lambda: self.on_menu_add_action_triggered(item)
            )
            menu.addAction(add_action)
        if isinstance(item, GalaxyTreeWidgetItem):
            add_star_action = Action("添加恒星条件")
            add_star_action.triggered.connect(
                lambda: self.on_menu_add_star_action_triggered(item)
            )
            menu.addAction(add_star_action)
        pos = QPoint(pos.x() + 10, pos.y() + 30)
        menu.exec(self.mapToGlobal(pos))

    def on_menu_del_action_triggered(self, item: TreeWidgetItem):
        item._on_del_button_clicked()

    def on_menu_add_star_action_triggered(self, item: GalaxyTreeWidgetItem):
        item.addStarLeaf()

    def on_menu_add_action_triggered(self, item: TreeWidgetItem):
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
