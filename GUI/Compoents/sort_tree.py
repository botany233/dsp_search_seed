__all__ = ["SortTree"]

from typing import Any
from PySide6.QtCore import Qt, QPoint, QSize
from PySide6.QtGui import QMouseEvent, QColor
from PySide6.QtWidgets import (
    QWidget,
    QTreeWidgetItem,
    QHBoxLayout,
    QHeaderView,
)

from qfluentwidgets import (
    TreeWidget,
    PushButton,
    RoundMenu,
    Action,
    BodyLabel,
    FluentIcon,
    SwitchButton,
    ToolButton,
    TransparentToolButton,
    setFont,
    setCustomStyleSheet,
    ToolTipFilter,
    ToolTipPosition
)

from .combox import AutoFixedComboBox
from .Widgets.line_edit import LimitLineEdit
from .multicombobox import MultiComboBox

from config import cfg
from logger import log
from config.cfg_dict_tying import (
    BondCondition,
    GalaxyCondition,
    PlanetCondition,
    StarCondition,
    VeinsName,
    BaseModel
)
from ..Messenger import SortTreeMessages
from .. import star_types, planet_types, singularity, liquid, dsp_level
from GUI.dsp_icons import AppIcons
from language import tr, tr_domain


# star_types = ["无限制"] + star_types
# planet_types = ["无限制"] + planet_types
# moon_parent_planet_types = ["无限制", "气态巨星", "高产气巨", "冰巨星"]
# moon_planet_types = ["无限制"] + [i for i in planet_types if i not in moon_parent_planet_types]
moon_parent_planet_types = ["气态巨星", "高产气巨", "冰巨星"]
moon_planet_types = [i for i in planet_types if i not in moon_parent_planet_types]
liquid = ["无限制"] + liquid
# singularity = ["无限制"] + singularity
moon_parent_singularity = [s for s in singularity if s != "卫星"]
moon_singularity = [s for s in singularity if s not in ["卫星", "多卫星", "潮汐锁定"]]
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

class TreeWidgetLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignRight)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self._placeholders: dict[int, TransparentToolButton] = {}
        self.addButton = ToolButton(AppIcons.STAR)
        self.addButton.setToolTip(tr("search.condition_tree.tooltips.add_star"))
        self.delButton = ToolButton(FluentIcon.DELETE)
        self.delButton.setToolTip(tr("search.condition_tree.tooltips.delete"))
        self.addPlanetButton = ToolButton(AppIcons.PLANET)
        self.addPlanetButton.setToolTip(tr("search.condition_tree.tooltips.add_planet"))

        self.addButton.installEventFilter(ToolTipFilter(self.addButton, showDelay=0, position=ToolTipPosition.BOTTOM_LEFT))
        self.delButton.installEventFilter(ToolTipFilter(self.delButton, showDelay=0, position=ToolTipPosition.BOTTOM_LEFT))
        self.addPlanetButton.installEventFilter(ToolTipFilter(self.addPlanetButton, showDelay=0, position=ToolTipPosition.BOTTOM_LEFT))

        self.mainLayout.addWidget(self.addButton)
        # self.mainLayout.addSpacing(5)
        self.mainLayout.addWidget(self.addPlanetButton)
        # self.mainLayout.addSpacing(5)
        self.mainLayout.addWidget(self.delButton)

    def add_placeholder_for(self, button: ToolButton):
        button_id = id(button)
        if button_id in self._placeholders:
            return

        index = self.mainLayout.indexOf(button)
        if index < 0:
            return

        placeholder = TransparentToolButton()
        placeholder.setEnabled(False)
        placeholder.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        placeholder.setFixedSize(button.sizeHint())
        qss = """TransparentToolButton{background: transparent;} TransparentToolButton:hover{background: transparent;}"""
        setCustomStyleSheet(placeholder, qss, qss)
        self.mainLayout.insertWidget(index, placeholder)
        button.setHidden(True)
        self._placeholders[button_id] = placeholder

class GalaxyTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj):
        super().__init__(root, config_obj, "galaxy_condition", root)

    def _on_add_planet_button_clicked(self):
        self.addPlanetLeaf()

    def _on_add_button_clicked(self):
        self.addStarLeaf()

    def _on_add_bond_button_clicked(self):
        self.addBondLeaf()

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

        self.manageButtons.addBondButton = ToolButton(AppIcons.BOND)
        self.manageButtons.addBondButton.setToolTip(tr("search.condition_tree.tooltips.add_bond"))
        self.manageButtons.addBondButton.installEventFilter(ToolTipFilter(self.manageButtons.addBondButton, showDelay=0, position=ToolTipPosition.BOTTOM_LEFT))
        self.manageButtons.mainLayout.insertWidget(2, self.manageButtons.addBondButton)
        self.manageButtons.addBondButton.clicked.connect(self._on_add_bond_button_clicked)

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
        index = 0
        while index < self.childCount() and not isinstance(self.child(index), BondTreeWidgetItem):
            index += 1
        self.insertChild(index, new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        return new_leaf

    def addBondLeaf(self, new_bond_condition: BondCondition|None = None) -> "BondTreeWidgetItem":
        if new_bond_condition is None:
            new_bond_condition = BondCondition()
            self.config_obj.bond_condition.append(new_bond_condition)
            cfg.save()

        new_leaf = BondTreeWidgetItem(self.root, new_bond_condition)
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
        self.manageButtons.add_placeholder_for(self.manageButtons.addButton)
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

from PySide6.QtCore import QObject, QEvent
class ToolTipFilterWithEgg(ToolTipFilter):
    def __init__(self, parent: QWidget, showDelay=300, position=ToolTipPosition.TOP):
        super().__init__(parent, showDelay, position)
        self.timer.setSingleShot(False)
        self._is_shown = False
        self.tooltip_num = 0
        self._mainWindow = None
        self._error = False

    def eventFilter(self, obj: QObject, e: QEvent) -> bool:
        if e.type() == QEvent.ToolTip:
            return True
        elif e.type() == QEvent.Hide:
            self.hideToolTip()
        elif e.type() == QEvent.Leave:
            self.tooltip_num = 0
            self.isEnter = False
            self.timer.stop()
            self.hideToolTip()
        elif e.type() == QEvent.Enter:
            if self._mainWindow is None and not self._error:
                parent = self.parent()  # type: QWidget
                loop_time = 0
                while parent.objectName() != "MainWindow":
                    parent = parent.parent()
                    if parent is None:
                        self._error = True
                        log.error("彩蛋获取主菜单失败!!")
                        break
                    if parent.objectName() == "MainWindow":
                        self._mainWindow = parent
                        break
                    loop_time += 1
                    if loop_time > 20:
                        self._error = True
                        log.error("彩蛋获取主菜单失败!!")
                        break
            self.isEnter = True
            parent = self.parent()  # type: QWidget
            if self._canShowToolTip():
                if self._tooltip is None:
                    self._tooltip = self._createToolTip()

                t = parent.toolTipDuration() if parent.toolTipDuration() > 0 else -1
                self._tooltip.setDuration(t)

                # show the tool tip after delay
                self.timer.start(self._tooltipDelay)
        elif e.type() == QEvent.MouseButtonPress:
            # self.hideToolTip()
            if self._is_shown:
                self.hideToolTip()
                self._tooltip.setText(tr("search.condition_tree.egg.warn"))
                parent = self.parent()  # type: QWidget
                self._tooltip.adjustPos(parent, self.position)
                self._tooltip.show()
                self._is_shown = True
                e.ignore()
                return True

        return super().eventFilter(obj, e)

    def _switchToolTipText(self):
        match self.tooltip_num:
            case 0: self._tooltip.setText(tr("search.condition_tree.egg.found"))
            case 1: self._tooltip.setText(tr("search.condition_tree.egg.button"))
            case 2: self._tooltip.setText(tr("search.condition_tree.egg.stop"))
            case _: self._tooltip.setText(tr("search.condition_tree.egg.consequence"))
        if self.tooltip_num >= 3:
            if (
                self._mainWindow is not None
                and (self._mainWindow.search_thread.isRunning()
                or self._mainWindow.viewerInterface.sort_thread.isRunning())
            ):
                self._tooltip.setText(tr("search.condition_tree.egg.forgive"))
    
    def showToolTip(self):
        """ show tool tip """
        if not self.isEnter:
            return
        parent = self.parent()  # type: QWidget
        if self.tooltip_num > 3:
            if (
                self._mainWindow is not None
                and not (self._mainWindow.search_thread.isRunning()
                or self._mainWindow.viewerInterface.sort_thread.isRunning())
            ):
                self._tooltip.setText(tr("search.condition_tree.egg.bye"))
                self._tooltip.adjustPos(parent, self.position)
                self._tooltip.show()
                from PySide6.QtWidgets import QApplication
                QApplication.processEvents()
                from time import sleep
                sleep(1)
                self._mainWindow.close()

            self.timer.stop()
            return
        if self._is_shown:
            self.hideToolTip()
            return

        parent = self.parent()  # type: QWidget
        self._switchToolTipText()
        self._tooltip.adjustPos(parent, self.position)
        self._tooltip.show()
        if self.tooltip_num >= 3:
            self.timer.stop()
            self.timer.start(self._tooltipDelay)
        self.tooltip_num += 1
        self._is_shown = True
    
    def hideToolTip(self):
        """ hide tool tip """
        if self._tooltip:
            self._tooltip.hide()
            self._is_shown = False

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

        self.manageButtons.add_placeholder_for(self.manageButtons.addButton)
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
        self.manageButtons.add_placeholder_for(self.manageButtons.addPlanetButton)
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

class SettingsTreeLeave(QWidget):
    def __init__(
        self,
        parent=None,
        buttonText: str = "矿物",
        items: dict[str, str] | None = None,
        config_obj: BaseModel|None = None,
        config_key: str | None = None,
        obj_name: str = "",
        description: str | None = None,
        display_title: str | None = None,
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
        self.display_title = display_title or obj_name or buttonText
        if obj_name != "":
            self.settingsButton.setObjectName(obj_name)
        else:
            self.settingsButton.setObjectName(buttonText)
        # self.setFocusPolicy(Qt.NoFocus)
        self.config_obj = config_obj
        self.config_key = config_key
        
        self._edited = False
        self.settingsButton.setProperty("edited", False)
        ltqss = """
        PushButton[edited="true"] {
            color:black;
        }
        PushButton[edited="false"] {
            color:grey;
        }
        """
        dkqss = """
        PushButton[edited="true"] {
            color:white;
        }
        PushButton[edited="false"] {
            color:grey;
        }
        """
        setCustomStyleSheet(self.settingsButton, ltqss, dkqss)
        if self._check_edited():
            self.edited = True
        else:
            self.edited = False

    def _check_edited(self):
        if self.config_obj is None or self.config_key is None:
            return False
        condition = getattr(self.config_obj, self.config_key)
        try:
            for item in condition:
                if item[1] != 0:
                    return True
            return False
        except Exception:
            return False
    
    @property
    def edited(self):
        return self._edited
    @edited.setter
    def edited(self, value: bool):
        self._edited = value
        self.settingsButton.setProperty("edited", value)
        if not value:
            color = QColor("grey")
            icon = FluentIcon.SETTING.colored(color, color)
            self.settingsButton.setIcon(icon)
        else:
            self.settingsButton.setIcon(FluentIcon.SETTING)
        self.settingsButton.style().unpolish(self.settingsButton)
        self.settingsButton.style().polish(self.settingsButton)
        self.settingsButton.update()

    def _createSettingsWindow(self):
        SortTreeMessages.CreateSettingsWindow.emit(self)


class GalaxyTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)

        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.veinsConditionButton = SettingsTreeLeave(
            buttonText=tr("search.condition_tree.buttons.veins_point"),
            config_obj=config_obj,
            config_key="veins_point_condition",
            items=VeinsName().model_dump(),
            obj_name="星区矿脉数量筛选",
            display_title=tr("search.condition_tree.settings.galaxy_veins_point_title"),
            description=tr("search.condition_tree.settings.galaxy_veins_point_description"),
        )
        self.veinsPointConditionButton = SettingsTreeLeave(
            buttonText=tr("search.condition_tree.buttons.veins_amount"),
            config_obj=config_obj,
            config_key="veins_amount_condition",
            items=VeinsName().model_dump(),
            obj_name="星区矿脉储量筛选",
            display_title=tr("search.condition_tree.settings.galaxy_veins_amount_title"),
            description=tr("search.condition_tree.settings.galaxy_veins_amount_description"),
        )
        self.mainLayout.addWidget(self.veinsConditionButton)
        self.mainLayout.addWidget(self.veinsPointConditionButton)


class StarTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj=config_obj)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.starTypeLabel = BodyLabel(tr("search.condition_tree.labels.star_type"))
        self.mainLayout.addWidget(self.starTypeLabel)
        # self.starTypeComboBox = AutoFixedComboBox(config_key="star_type")
        self.starTypeComboBox = MultiComboBox("star_type", self.config_obj, domain="star_types")
        self.starTypeComboBox.addItems(star_types)
        self.mainLayout.addWidget(self.starTypeComboBox)
        self.luminosityLabel = BodyLabel(tr("search.condition_tree.labels.lumino"))
        self.mainLayout.addWidget(self.luminosityLabel)
        self.luminosityLineEdit = LimitLineEdit("lumino_level", self.config_obj, "float", 0, default_value=0.0)
        self.mainLayout.addWidget(self.luminosityLineEdit)

        self.distanceLabel = BodyLabel(tr("search.condition_tree.labels.distance"))
        self.mainLayout.addWidget(self.distanceLabel)
        self.distanceLineEdit = LimitLineEdit("distance_level", self.config_obj, "float", 0, default_value=-1.0)
        self.mainLayout.addWidget(self.distanceLineEdit)

        self.hitStarNumLabel = BodyLabel(tr("search.condition_tree.labels.satisfy_num"))
        self.mainLayout.addWidget(self.hitStarNumLabel)
        self.hitStarNumLineEdit = LimitLineEdit("satisfy_num", self.config_obj, min_value=1, max_value=65535, default_value=1, empty_invisible=False)
        self.mainLayout.addWidget(self.hitStarNumLineEdit)
        self.distanceLineEdit.setMaximumHeight(28)
        self.distanceLineEdit.setFixedHeight(28)
        self.hitStarNumLineEdit.setMaximumHeight(28)
        self.hitStarNumLineEdit.setFixedHeight(28)
        self.luminosityLineEdit.setMaximumHeight(28)
        self.luminosityLineEdit.setFixedHeight(28)
        self.veinsConditionButton = SettingsTreeLeave(
            buttonText=tr("search.condition_tree.buttons.veins_point"),
            config_obj=config_obj,
            config_key="veins_point_condition",
            items=VeinsName().model_dump(),
            obj_name="恒星系矿脉数量筛选",
            display_title=tr("search.condition_tree.settings.star_veins_point_title"),
            description=tr("search.condition_tree.settings.star_veins_point_description"),
        )
        self.veinsPointConditionButton = SettingsTreeLeave(
            buttonText=tr("search.condition_tree.buttons.veins_amount"),
            config_obj=config_obj,
            config_key="veins_amount_condition",
            items=VeinsName().model_dump(),
            obj_name="恒星系矿脉储量筛选",
            display_title=tr("search.condition_tree.settings.star_veins_amount_title"),
            description=tr("search.condition_tree.settings.star_veins_amount_description"),
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
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.planetTypeLabel = BodyLabel(tr("search.condition_tree.labels.planet_type"))
        self.mainLayout.addWidget(self.planetTypeLabel)
        # self.planetTypeComboBox = AutoFixedComboBox(config_key="planet_type")
        self.planetTypeComboBox = MultiComboBox("planet_type", self.config_obj, domain="planet_types")
        self.planetTypeComboBox.addItems(planet_types)
        self.mainLayout.addWidget(self.planetTypeComboBox)
        self.singularityLabel = BodyLabel(tr("search.condition_tree.labels.singularity"))
        self.mainLayout.addWidget(self.singularityLabel)
        # self.singularityComboBox = AutoFixedComboBox(config_key="singularity")
        self.singularityComboBox = MultiComboBox("singularity", self.config_obj, domain="singularity")
        self.singularityComboBox.addItems(singularity)
        self.mainLayout.addWidget(self.singularityComboBox)
        self.liquidLabel = BodyLabel(tr("search.condition_tree.labels.liquid"))
        self.mainLayout.addWidget(self.liquidLabel)
        self.liquidComboBox = AutoFixedComboBox(config_key="liquid_type", domain="liquids")
        self.liquidComboBox.addItems(liquid)
        self.mainLayout.addWidget(self.liquidComboBox)
        self.dspLevelLabel = BodyLabel(tr("search.condition_tree.labels.dsp_level"))
        self.mainLayout.addWidget(self.dspLevelLabel)
        self.dspLevelComboBox = AutoFixedComboBox(config_key="dsp_level", domain="dsp_levels")
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
        self.hitStarNumLabel = BodyLabel(tr("search.condition_tree.labels.satisfy_num"))
        self.mainLayout.addWidget(self.hitStarNumLabel)
        self.hitStarNumLineEdit = LimitLineEdit("satisfy_num", self.config_obj, min_value=1, max_value=65535, default_value=1, empty_invisible=False)
        self.mainLayout.addWidget(self.hitStarNumLineEdit)

        self.hitStarNumLineEdit.setMaximumHeight(28)
        self.hitStarNumLineEdit.setFixedHeight(28)

        self.veinsConditionButton = SettingsTreeLeave(
            buttonText=tr("search.condition_tree.buttons.veins_point"),
            config_obj=config_obj,
            config_key="veins_point_condition",
            items=VeinsName().model_dump(),
            obj_name="星球矿脉数量筛选",
            display_title=tr("search.condition_tree.settings.planet_veins_point_title"),
            description=tr("search.condition_tree.settings.planet_veins_point_description"),
        )
        self.veinsPointConditionButton = SettingsTreeLeave(
            buttonText=tr("search.condition_tree.buttons.veins_amount"),
            config_obj=config_obj,
            config_key="veins_amount_condition",
            items=VeinsName().model_dump(),
            obj_name="星球矿脉储量筛选",
            display_title=tr("search.condition_tree.settings.planet_veins_amount_title"),
            description=tr("search.condition_tree.settings.planet_veins_amount_description"),
        )
        self.mainLayout.addWidget(self.veinsConditionButton)
        self.mainLayout.addWidget(self.veinsPointConditionButton)

    def load_config(self):
        self.planetTypeComboBox.load_config()
        self.singularityComboBox.load_config()
        self.liquidComboBox.load_config()
        self.dspLevelComboBox.load_config()


class BondTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None, bond_item: "BondTreeWidgetItem" = None):
        super().__init__(parent, config_obj=config_obj)
        self.bond_item = bond_item
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)

        self.distanceLabel = BodyLabel(tr("search.condition_tree.labels.distance"))
        self.mainLayout.addWidget(self.distanceLabel)
        self.distanceLineEdit = LimitLineEdit("distance", self.config_obj, "float", 0, default_value=-1.0)
        self.mainLayout.addWidget(self.distanceLineEdit)

        self.satisfyNumLabel = BodyLabel(tr("search.condition_tree.labels.bond_satisfy_num"))
        self.mainLayout.addWidget(self.satisfyNumLabel)
        self.satisfyNumLineEdit = LimitLineEdit("satisfy_num", self.config_obj, min_value=1, max_value=65535, default_value=1, empty_invisible=False)
        self.mainLayout.addWidget(self.satisfyNumLineEdit)

        self.con1Label = BodyLabel(tr("search.condition_tree.labels.bond_con1"))
        self.mainLayout.addWidget(self.con1Label)
        self.con1TypeSwitch = self._create_endpoint_switch(1)
        self.mainLayout.addWidget(self.con1TypeSwitch)

        self.con2Label = BodyLabel(tr("search.condition_tree.labels.bond_con2"))
        self.mainLayout.addWidget(self.con2Label)
        self.con2TypeSwitch = self._create_endpoint_switch(2)
        self.mainLayout.addWidget(self.con2TypeSwitch)

        self.distanceLineEdit.setMaximumHeight(28)
        self.distanceLineEdit.setFixedHeight(28)
        self.satisfyNumLineEdit.setMaximumHeight(28)
        self.satisfyNumLineEdit.setFixedHeight(28)

    def _create_endpoint_switch(self, endpoint: int) -> SwitchButton:
        type_switch = SwitchButton()
        star_text = tr("search.condition_tree.bond_switch.star")
        planet_text = tr("search.condition_tree.bond_switch.planet")
        type_switch.setOffText(star_text)
        type_switch.setOnText(planet_text)
        type_switch.label.setFixedWidth(max(
            type_switch.label.fontMetrics().horizontalAdvance(star_text),
            type_switch.label.fontMetrics().horizontalAdvance(planet_text),
        ) + 6)
        type_switch.setChecked(getattr(self.config_obj, f"con{endpoint}_is_planet"))
        type_switch.checkedChanged.connect(lambda checked, endpoint=endpoint: self._on_type_changed(endpoint, checked))
        return type_switch

    def _on_type_changed(self, endpoint: int, checked: bool):
        if self.bond_item is not None:
            self.bond_item.set_endpoint_is_planet(endpoint, checked)
        else:
            setattr(self.config_obj, f"con{endpoint}_is_planet", checked)
            cfg.save()


class BondEndpointTreeLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None, endpoint_item: "BondEndpointTreeWidgetItem" = None):
        super().__init__(parent, config_obj=config_obj)
        self.endpoint_item = endpoint_item
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.mainLayout.setSpacing(0)

        self.conditionWidget: StarTreeLeave | PlanetTreeLeave | None = None
        self.switch_condition_widget()

    def switch_condition_widget(self):
        if self.conditionWidget is not None:
            self.mainLayout.removeWidget(self.conditionWidget)
            self.conditionWidget.deleteLater()

        if self.endpoint_item.active_is_planet:
            self.conditionWidget = PlanetTreeLeave(config_obj=self.endpoint_item.active_config)
        else:
            self.conditionWidget = StarTreeLeave(config_obj=self.endpoint_item.active_config)
        self.conditionWidget.load_config()
        self.conditionWidget.hitStarNumLabel.setText(tr("search.condition_tree.labels.max_connection_num"))
        self.mainLayout.addWidget(self.conditionWidget)


class BondTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", config_obj: BondCondition):
        super().__init__(root, config_obj, "bond_condition")

    def add_widgets(self):
        self.leaf = BondTreeLeave(config_obj=self.config_obj, bond_item=self)
        self.root.setItemWidget(self, 1, self.leaf)

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.add_placeholder_for(self.manageButtons.addButton)
        self.manageButtons.add_placeholder_for(self.manageButtons.addPlanetButton)
        self.manageButtons.delButton.clicked.connect(self._on_del_button_clicked)

        if self.childCount() == 0:
            self.addEndpointLeaf(1)
            self.addEndpointLeaf(2)
        self.setExpanded(True)

    def addEndpointLeaf(self, endpoint: int) -> "BondEndpointTreeWidgetItem":
        new_leaf = BondEndpointTreeWidgetItem(self.root, self.config_obj, endpoint)
        self.addChild(new_leaf)
        new_leaf.add_widgets()
        return new_leaf

    def getEndpointLeaf(self, endpoint: int) -> "BondEndpointTreeWidgetItem | None":
        for index in range(self.childCount()):
            child = self.child(index)
            if isinstance(child, BondEndpointTreeWidgetItem) and child.endpoint == endpoint:
                return child
        return None

    def set_endpoint_is_planet(self, endpoint: int, is_planet: bool):
        endpoint_leaf = self.getEndpointLeaf(endpoint)
        if endpoint_leaf is not None:
            endpoint_leaf.set_is_planet(is_planet)
            return
        setattr(self.config_obj, f"con{endpoint}_is_planet", is_planet)
        cfg.save()

    def _on_del_button_clicked(self):
        if self.parent() is None:
            return

        belong_list = self.parent().config_obj.bond_condition
        delete_index = -1
        for i, bond_condition in enumerate(belong_list):
            if bond_condition is self.config_obj:
                delete_index = i
                break
        if delete_index == -1:
            for i, bond_condition in enumerate(belong_list):
                if bond_condition == self.config_obj:
                    delete_index = i
                    break
        if delete_index == -1:
            raise Exception("bond condition not found!")

        belong_list.pop(delete_index)
        cfg.save()
        self.parent().removeChild(self)


class BondEndpointTreeWidgetItem(TreeWidgetItem):
    def __init__(self, root: "SortTree", bond_config_obj: BondCondition, endpoint: int):
        self.bond_config_obj = bond_config_obj
        self.endpoint = endpoint
        self.has_moon = False
        super().__init__(root, self.active_config, f"con{endpoint}")
        self._refresh_text()

    @property
    def active_is_planet(self) -> bool:
        return getattr(self.bond_config_obj, f"con{self.endpoint}_is_planet")

    @property
    def active_config(self) -> PlanetCondition | StarCondition:
        if self.active_is_planet:
            return getattr(self.bond_config_obj, f"con{self.endpoint}_planet")
        return getattr(self.bond_config_obj, f"con{self.endpoint}_star")

    def add_widgets(self):
        self.leaf = BondEndpointTreeLeave(config_obj=self.bond_config_obj, endpoint_item=self)
        self.root.setItemWidget(self, 1, self.leaf)

        self.manageButtons = TreeWidgetLeave()
        self.root.setItemWidget(self, 2, self.manageButtons)
        self.manageButtons.add_placeholder_for(self.manageButtons.addButton)
        self.manageButtons.add_placeholder_for(self.manageButtons.delButton)
        self.manageButtons.addPlanetButton.clicked.connect(self._on_add_button_clicked)
        self._sync_manage_buttons()
        self._populate_children()
        self.setExpanded(True)

    def setData(self, column: int, role: int, value: Any) -> None:
        self.config_obj = self.active_config
        return super().setData(column, role, value)

    def set_is_planet(self, is_planet: bool):
        if self.active_is_planet == is_planet:
            return
        setattr(self.bond_config_obj, f"con{self.endpoint}_is_planet", is_planet)
        cfg.save()

        self._clear_children()
        self.config_obj = self.active_config
        self.has_moon = False
        self._refresh_text()
        self._update_check_state_from_config()
        self.leaf.switch_condition_widget()
        self._sync_manage_buttons()
        self._populate_children()
        self.setExpanded(True)

    def _refresh_text(self):
        self.setText(0, tr_domain("condition_names", self.active_config.custom_name))

    def _sync_manage_buttons(self):
        if self.active_is_planet:
            self.manageButtons.addPlanetButton.setIcon(AppIcons.MOON)
            self.manageButtons.addPlanetButton.setToolTip(tr("search.condition_tree.tooltips.add_moon"))
        else:
            self.manageButtons.addPlanetButton.setIcon(AppIcons.PLANET)
            self.manageButtons.addPlanetButton.setToolTip(tr("search.condition_tree.tooltips.add_planet"))

    def _clear_children(self):
        while self.childCount() > 0:
            self.takeChild(0)

    def _populate_children(self):
        if self.active_is_planet:
            for moon_condition in self.active_config.moon_conditions:
                self.addMoonLeaf(moon_condition)
            return

        for planet_condition in self.active_config.planet_condition:
            planet_leaf = self.addPlanetLeaf(planet_condition)
            for moon_condition in planet_condition.moon_conditions:
                planet_leaf.addMoonLeaf(moon_condition)

    def _on_add_button_clicked(self):
        if self.active_is_planet:
            self.addMoonLeaf()
        else:
            self.addPlanetLeaf()

    def addPlanetLeaf(self, new_planet_condition: PlanetCondition|None = None) -> "PlanetTreeWidgetItem":
        if self.active_is_planet:
            return self.addMoonLeaf(new_planet_condition)
        if new_planet_condition is None:
            new_planet_condition = PlanetCondition()
            self.active_config.planet_condition.append(new_planet_condition)
            cfg.save()

        new_leaf = PlanetTreeWidgetItem(self.root, new_planet_condition)
        self.addChild(new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        return new_leaf

    def addMoonLeaf(self, new_planet_condition: PlanetCondition|None = None) -> "MoonTreeWidgetItem":
        if not self.active_is_planet:
            return None
        if new_planet_condition is None:
            new_planet_condition = PlanetCondition()
            new_planet_condition.custom_name = "卫星条件"
            self.active_config.moon_conditions.append(new_planet_condition)
            cfg.save()

        new_leaf = MoonTreeWidgetItem(self.root, new_planet_condition)
        self.addChild(new_leaf)
        new_leaf.add_widgets()
        self.setExpanded(True)
        self._switch_moons_planet()
        return new_leaf

    def _planet_leave(self) -> PlanetTreeLeave | None:
        if not hasattr(self, "leaf") or not isinstance(self.leaf.conditionWidget, PlanetTreeLeave):
            return None
        return self.leaf.conditionWidget

    def _switch_moons_planet(self):
        if self.has_moon:
            return
        planet_leave = self._planet_leave()
        if planet_leave is None:
            return
        planet_leave.planetTypeComboBox.clear()
        planet_leave.planetTypeComboBox.addItems(moon_parent_planet_types)
        planet_leave.planetTypeComboBox.load_config()
        planet_leave.singularityComboBox.clear()
        planet_leave.singularityComboBox.addItems(moon_parent_singularity)
        planet_leave.singularityComboBox.load_config()
        self.has_moon = True

    def _switch_no_moons_planet(self):
        if not self.has_moon:
            return
        planet_leave = self._planet_leave()
        if planet_leave is None:
            return
        planet_leave.planetTypeComboBox.clear()
        planet_leave.planetTypeComboBox.addItems(planet_types)
        planet_leave.planetTypeComboBox.load_config()
        planet_leave.singularityComboBox.clear()
        planet_leave.singularityComboBox.addItems(singularity)
        planet_leave.singularityComboBox.load_config()
        self.has_moon = False

    def _on_del_button_clicked(self):
        return

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

        self.setColumnWidth(0, 186)
        self.setColumnWidth(1, 550)
        self.setColumnWidth(2, 130)

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
        if not isinstance(item, (GalaxyTreeWidgetItem, BondEndpointTreeWidgetItem)):
            del_action = Action(tr("search.condition_tree.menu.delete"))
            del_action.triggered.connect(
                lambda: self.on_menu_del_action_triggered(item)
            )
            menu.addAction(del_action)

        if isinstance(item, BondEndpointTreeWidgetItem):
            if item.active_is_planet:
                add_moon_action = Action(tr("search.condition_tree.menu.add_moon"))
                add_moon_action.triggered.connect(
                    lambda: self.on_menu_add_moon_action_triggered(item)
                )
                menu.addAction(add_moon_action)
            else:
                add_action = Action(tr("search.condition_tree.menu.add_planet"))
                add_action.triggered.connect(
                    lambda: self.on_menu_add_action_triggered(item)
                )
                menu.addAction(add_action)
        elif not isinstance(item, (PlanetTreeWidgetItem, MoonTreeWidgetItem, BondTreeWidgetItem)):
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

            add_bond_action = Action(tr("search.condition_tree.menu.add_bond"))
            add_bond_action.triggered.connect(
                lambda: self.on_menu_add_bond_action_triggered(item)
            )
            menu.addAction(add_bond_action)

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

    def on_menu_add_moon_action_triggered(self, item: PlanetTreeWidgetItem|BondEndpointTreeWidgetItem):
        item.addMoonLeaf()

    def on_menu_add_star_action_triggered(self, item: GalaxyTreeWidgetItem):
        item.addStarLeaf()

    def on_menu_add_bond_action_triggered(self, item: GalaxyTreeWidgetItem):
        item.addBondLeaf()

    def on_menu_add_action_triggered(self, item: GalaxyTreeWidgetItem|StarTreeWidgetItem|BondEndpointTreeWidgetItem):
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
