__all__ = ["SortTree"]

from typing import Any
from PySide6.QtCore import QAbstractItemModel, Qt, QPoint, QSize, Signal
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
    SwitchButton,
)

from .combox import AutoFixedComboBox
from .Widgets.line_edit import ConfigLineEdit
from .Widgets.button import ConfigSwitchButton


from config import cfg
from config.cfg_dict_tying import (
    GalaxyCondition,
    PlanetCondition,
    StarSystemCondition,
    VeinsName,
    CelestialCondition,
)
from ..Messenger import SortTreeMessages
from .. import star_types, planet_types, singularity, liquid

star_types = ["无 / 任意"] + star_types
planet_types = ["无 / 任意"] + planet_types
liquid = ["无 / 任意"] + liquid
singularity = ["无 / 任意"] + singularity

class ConditionBase(QWidget):
    def __init__(self, parent=None, config_obj=None):
        self.config_obj = config_obj
        super().__init__(parent)

    def load_config(self):
        pass

class TreeWidgetItem(QTreeWidgetItem):
    def __init__(self, *args, root: "SortTree", config_obj: Any, **kwargs):
        self.root = root
        self.config_obj = config_obj
        self.flag = True
        super().__init__(*args, **kwargs)
        self.setSizeHint(0, QSize(0, 40))
        self.setFlags(self.flags() | Qt.ItemIsEditable)
        self._add_widgets_()
        self._update_check_state_from_config()

    def _add_widgets_(self):
        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.addButton.clicked.connect(self._on_add_button_clicked)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

    def _update_check_state_from_config(self):
        checked = getattr(self.config_obj, "checked", -1)
        match checked:
            case 0:
                self.setCheckState(0, Qt.CheckState.Unchecked)
            case 1:
                self.setCheckState(0, Qt.CheckState.Checked)
            case _:
                setattr(self.config_obj, "checked", 1)
                self.setCheckState(0, Qt.CheckState.Checked)

    def setData(self, column: int, role: int, value: Any) -> None:
        if column == 0 and role == Qt.CheckStateRole:
            setattr(self.config_obj, "checked", value)
            cfg.save()
            super().setData(column, role, value)
            return

        if column == 0 and role == Qt.EditRole:
            # 修改名称
            if self.parent() is None:
                setattr(self.config_obj, "custom_name", value)
                cfg.save()
            else:
                try:
                    setattr(self.config_obj, "custom_name", value)
                    cfg.save()
                except Exception as e:
                    print(f"设置配置项名称失败: {e}")

        return super().setData(column, role, value)

    def GetIndex(self) -> int:
        if self.parent() is None:
            return self.root.indexOfTopLevelItem(self)
        return self.parent().indexOfChild(self)

    def resetChildIndex(self, child=None):
        if child is not None:
            self.removeChild(child)

        for i in range(self.childCount()):
            child = self.child(i)
            child.index = i

    def _on_add_button_clicked(self):
        if hasattr(self, "addLeaf"):
            leaf = self.addLeaf()
            while leaf is not None:
                leaf = leaf.addLeaf()

    def _on_del_button_clicked(self):
        if self.parent() is None:
            return
        try:
            a = getattr(self.branch.config_obj, self.config_key)
            a.pop(self.index)
            cfg.save()

        except Exception as e:
            print(f"删除配置项失败: {e}")

        self.parent().resetChildIndex(self)


class TreeWidgetLeave(ConditionBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj)
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


class GalaxyTreeWidgetItem(TreeWidgetItem):
    """银河系条件项"""

    def __init__(self, root: "SortTree", *args, **kwargs):
        text = "银河系条件"
        self.index = root.topLevelItemCount()

        self.config_key = "conditions"
        try:
            self.config_obj = cfg.config.conditions
        except AttributeError:
            self.config_obj = GalaxyCondition()
            setattr(self.config_obj, "custom_name", text)
            cfg.save()
        else:
            try:
                text = getattr(self.config_obj, "custom_name")
            except AttributeError:
                text = "银河系条件"
                self.config_obj = GalaxyCondition()
                setattr(self.config_obj, self.config_key, self.config_obj)
                cfg.save()
        texts = [text]
        super().__init__(
            root, texts, *args, root=root, config_obj=self.config_obj, **kwargs
        )
        self.manageButtons.delButton.setToolTip("重置该项及其子项")
        self.manageButtons.delButton.clicked.connect(self.reset)
        self.__bind__widgets__()

    def reset(self):
        for i in range(self.childCount() - 1, -1, -1):
            child = self.child(i)
            self.removeChild(child)
        self.config_obj = GalaxyCondition()
        cfg.config.conditions = self.config_obj
        cfg.save()
        leaf = self.addLeaf()
        while leaf is not None:
            leaf = leaf.addLeaf()

    def __bind__widgets__(self):
        self.leaf = GalaxyTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)

    def addLeaf(self) -> "SystemTreeWidgetItem":
        leaf = SystemTreeWidgetItem(self.root, self)
        self.addChild(leaf)
        self.setExpanded(True)
        return leaf


class SystemTreeWidgetItem(TreeWidgetItem):
    """恒星系条件项"""

    def __init__(self, root: "SortTree", branch: GalaxyTreeWidgetItem, *args, **kwargs):
        self.index = branch.childCount()
        text = f"恒星系条件{branch.childCount() + 1}"

        self.branch = branch

        self.config_key = "star_system_conditions"
        try:
            self.config_obj = getattr(self.branch.config_obj, self.config_key)[
                self.index
            ]
        except IndexError:
            a: list = getattr(self.branch.config_obj, self.config_key)
            a.append(StarSystemCondition())
            cfg.save()
            self.config_obj = getattr(self.branch.config_obj, self.config_key)[
                self.index
            ]
        else:
            text = getattr(self.config_obj, "custom_name")

        texts = [text]
        super().__init__(
            branch, texts, *args, root=root, config_obj=self.config_obj, **kwargs
        )
        self.__bind__widgets__()

    def __bind__widgets__(self):
        self.leaf = SystemTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)
        self.leaf.load_config()
        pass

    def addLeaf(self) -> "PlanetTreeWidgetItem":
        leaf = PlanetTreeWidgetItem(self.root, self)
        self.addChild(leaf)
        self.setExpanded(True)
        return leaf


class PlanetTreeWidgetItem(TreeWidgetItem):
    """行星条件项"""

    def __init__(self, root: "SortTree", branch: SystemTreeWidgetItem, *args, **kwargs):
        self.index = branch.childCount()
        text = f"行星条件{branch.childCount() + 1}"

        self.branch = branch

        self.config_key = "planet_conditions"

        try:
            self.config_obj = getattr(self.branch.config_obj, self.config_key)[
                self.index
            ]

        except IndexError:
            a: list = getattr(self.branch.config_obj, self.config_key)
            a.append(PlanetCondition())
            cfg.save()
            self.config_obj = getattr(self.branch.config_obj, self.config_key)[
                self.index
            ]
        else:
            text = getattr(self.config_obj, "custom_name")
        texts = [text]
        super().__init__(
            branch, texts, *args, root=root, config_obj=self.config_obj, **kwargs
        )
        self.manageButtons.addButton.setHidden(True)
        self.manageButtons.adjust_del_button()

        self.__bind__widgets__()

    def __bind__widgets__(self):
        self.leaf = PlanetTreeLeave(config_obj=self.config_obj)
        self.root.setItemWidget(self, 1, self.leaf)
        self.leaf.load_config()

    def addLeaf(self) -> None:
        return


class SettingsTreeLeave(QWidget):
    def __init__(
        self,
        parent=None,
        buttonText: str = "矿物筛选",
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


class GalaxyTreeLeave(ConditionBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        celestial_condition = dict(
            zip(
                CelestialCondition.get_field_mapping().values(),
                CelestialCondition.get_field_mapping().keys(),
            )
        )
        self.celestialConditionButton = SettingsTreeLeave(
            buttonText="天体筛选 (银河系)",
            config_obj=config_obj,
            config_key="celestial_condition",
            items=celestial_condition,
            obj_name="银河系天体筛选",
            description="设置银河系筛选条件, 内容为银河系内最少有多少对应天体, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.celestialConditionButton)
        self.veinsConditionButton = SettingsTreeLeave(
            buttonText="矿物筛选 (银河系)",
            config_obj=config_obj,
            config_key="veins_condition",
            items=VeinsName().model_dump(),
            obj_name="银河系矿物筛选",
            description="设置银河系筛选条件, 内容为银河系内最少有多少对应矿物, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.veinsConditionButton)


class SystemTreeLeave(ConditionBase):
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
        self.luminosityLineEdit = ConfigLineEdit(
            config_key="lumino_level", config_obj=self.config_obj, type_input="float"
        )
        self.mainLayout.addWidget(self.luminosityLineEdit)

        self.distanceLabel = BodyLabel("最远距离")
        self.mainLayout.addWidget(self.distanceLabel)
        self.distanceLineEdit = ConfigLineEdit(
            config_key="distance_level", config_obj=self.config_obj, type_input="float"
        )
        self.mainLayout.addWidget(self.distanceLineEdit)

        self.hitStarNumLabel = BodyLabel("符合的恒星数")
        self.mainLayout.addWidget(self.hitStarNumLabel)
        self.hitStarNumLineEdit = ConfigLineEdit(
            config_key="distance_hited_star_num", config_obj=self.config_obj
        )
        self.mainLayout.addWidget(self.hitStarNumLineEdit)
        self.distanceLineEdit.setMaximumHeight(28)
        self.distanceLineEdit.setFixedHeight(28)
        self.hitStarNumLineEdit.setMaximumHeight(28)
        self.hitStarNumLineEdit.setFixedHeight(28)
        self.luminosityLineEdit.setMaximumHeight(28)
        self.luminosityLineEdit.setFixedHeight(28)
        celestial_condition = dict(
            zip(
                CelestialCondition.get_field_mapping().values(),
                CelestialCondition.get_field_mapping().keys(),
            )
        )
        self.celestialConditionButton = SettingsTreeLeave(
            buttonText="天体筛选",
            config_obj=config_obj,
            config_key="celestial_condition",
            items=celestial_condition,
            obj_name="恒星系天体筛选",
            description="设置天体筛选条件, 内容为某个恒星系内最少有多少对应天体, 不填写则不限制",
        )
        self.settingsButton = SettingsTreeLeave(
            buttonText="矿物筛选",
            config_obj=config_obj,
            config_key="veins_condition",
            items=VeinsName().model_dump(),
            obj_name="恒星系矿物筛选",
            description="设置恒星系筛选条件, 内容为某个恒星系内最少有多少对应矿物, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.celestialConditionButton)
        self.mainLayout.addWidget(self.settingsButton)
        self.settingsButton.setToolTip("设置恒星系筛选条件")

    def load_config(self):
        self.starTypeComboBox.load_config()


class PlanetTreeLeave(ConditionBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignLeft)
        self.mainLayout.setContentsMargins(5, 0, 5, 0)
        self.planetTypeLabel = BodyLabel("行星类型")
        self.mainLayout.addWidget(self.planetTypeLabel)
        self.planetTypeComboBox = AutoFixedComboBox(config_key="planet_type")
        self.planetTypeComboBox.addItems(planet_types)
        self.mainLayout.addWidget(self.planetTypeComboBox)
        self.singularityLabel = BodyLabel("特点")
        self.mainLayout.addWidget(self.singularityLabel)
        self.singularityComboBox = AutoFixedComboBox(config_key="singularity")
        self.singularityComboBox.addItems(singularity)
        self.mainLayout.addWidget(self.singularityComboBox)
        self.liquidLabel = BodyLabel("液体类型")
        self.mainLayout.addWidget(self.liquidLabel)
        self.liquidComboBox = AutoFixedComboBox(config_key="liquid_type")
        self.liquidComboBox.addItems(liquid)
        self.mainLayout.addWidget(self.liquidComboBox)
        self.fullCoverdPlanetSwitch = ConfigSwitchButton(
            "是<u><i>否</i></u>为全包星", indicatorPos=1
        )
        self.fullCoverdPlanetSwitch.setOnText("<u><i>是</i></u>否为全包星")
        self.mainLayout.addWidget(self.fullCoverdPlanetSwitch)
        self.fullCoverdPlanetSwitch.set_config(
            config_obj=config_obj, config_key="full_coverd_dsp"
        )

        self.settingsButton = SettingsTreeLeave(
            buttonText="矿物筛选",
            config_obj=config_obj,
            config_key="veins_condition",
            items=VeinsName().model_dump(),
            obj_name="行星矿物筛选",
            description="设置星体筛选条件, 内容为某个行星内最少有多少对应矿物, 不填写则不限制",
        )
        self.mainLayout.addWidget(self.settingsButton)

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
            if item is self.leaf:
                del_action = Action("重置项")
            else:
                del_action = Action("删除项")
            del_action.triggered.connect(
                lambda: self.on_menu_del_action_triggered(item)
            )
            menu.addAction(del_action)
        if not isinstance(item, PlanetTreeWidgetItem):
            add_action = Action("添加子项")
            add_action.triggered.connect(
                lambda: self.on_menu_add_action_triggered(item)
            )
            menu.addAction(add_action)
        pos = QPoint(pos.x() + 10, pos.y() + 30)
        menu.exec(self.mapToGlobal(pos))

    def on_menu_del_action_triggered(self, item: QTreeWidgetItem):
        if item is self.leaf:
            return
        if item.parent() is None:
            self.takeTopLevelItem(self.indexOfTopLevelItem(item))
            return
        item.parent().removeChild(item)

    def on_menu_add_action_triggered(self, item: QTreeWidgetItem):
        if item is None:
            leaf = self.addLeaf()
            while leaf is not None:
                leaf = leaf.addLeaf()
        else:
            if hasattr(item, "addLeaf"):
                leaf = item.addLeaf()
                while leaf is not None:
                    leaf = leaf.addLeaf()

    def addLeaf(self) -> GalaxyTreeWidgetItem:
        if self.leaf is not None:
            return  # type: ignore

        self.leaf = GalaxyTreeWidgetItem(self)
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
