from PySide6.QtCore import Qt, QObject, QEvent
from PySide6.QtGui import QColor
from PySide6.QtWidgets import QWidget, QHBoxLayout
from qfluentwidgets import (
    PushButton, BodyLabel, FluentIcon, ToolButton, setFont,
    setCustomStyleSheet, ToolTipFilter, ToolTipPosition, CheckBox,
)

from ...Compoents.combox import AutoFixedComboBox
from ...Compoents.Widgets.line_edit import LimitLineEdit
from ...Compoents.multicombobox import MultiComboBox
from ...Messenger import SortTreeMessages
from ... import star_types, planet_types, singularity, liquid, dsp_level
from ...dsp_icons import AppIcons
from config.cfg_dict_tying import VeinsName, BaseModel
from logger import log
from language import tr


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

class TreeWidgetLeave(LeaveBase):
    def __init__(self, parent=None, config_obj=None):
        super().__init__(parent, config_obj)
        self.mainLayout = QHBoxLayout(self)
        self.mainLayout.setAlignment(Qt.AlignRight)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
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
                and (self._mainWindow.searchInterface.get_search_task().isRunning()
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
                and not (self._mainWindow.searchInterface.get_search_task().isRunning()
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
        self.invertCheckBox = CheckBox(tr("search.condition_tree.invert_all"))
        self.invertCheckBox.setToolTip(tr("search.condition_tree.invert_all_description"))
        self.invertCheckBox.installEventFilter(ToolTipFilter(self.invertCheckBox, showDelay=0))
        self.invertCheckBox.setChecked(not self.config_obj.valid_state)
        self.invertCheckBox.toggled.connect(self._on_invert_changed)
        self.mainLayout.addWidget(self.invertCheckBox)

    def _on_invert_changed(self, checked: bool):
        from config import cfg

        self.config_obj.valid_state = not checked
        cfg.save()


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


