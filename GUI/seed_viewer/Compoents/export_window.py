from PySide6.QtCore import QEvent, Qt
from PySide6.QtGui import QColor, QResizeEvent
from PySide6.QtWidgets import QHBoxLayout, QFrame, QVBoxLayout, QDialog, QGridLayout, QWidget
from qfluentwidgets import (
    MessageBox,
    MaskDialogBase,
    Dialog,
    BodyLabel,
    FluentStyleSheet,
    TitleLabel,
    CheckBox,
    PrimaryPushButton,
    PushButton,
    isDarkTheme,
)
from GUI.Compoents import ConfigCheckBox
from config import cfg

class Ui_MessageBox:
    """ Ui of message box """

    def __init__(self, *args, **kwargs):
        pass

    def _setUpUi(self, parent):

        self.mainLayout = QVBoxLayout(parent)

        self._initWidget()

    def _initWidget(self):
        self._setQss()
        self._initLayout()

    def _initLayout(self):
        self.mainLayout.setSpacing(0)
        self.mainLayout.setContentsMargins(10, 10, 10, 10)
        self.mainLayout.setSizeConstraint(QVBoxLayout.SetMinimumSize)

    def set_border(self, widget: QFrame, radius: int = 8):
        qss = f""".QFrame {{border-radius: {radius}px; border: 1px solid --border-color;}}"""
        widget.setStyleSheet(qss.replace("--border-color", "rgb(200, 200, 200)" if not isDarkTheme() else "rgb(80, 80, 80)"))

    def _setQss(self):
        qss = """
QFrame {
    border-radius: 8px;
    background-color: --background-color;
}"""    
        qss = qss.replace("--background-color", "rgb(255, 255, 255)" if not isDarkTheme() else "rgb(43, 43, 43)")
        self.setStyleSheet(qss)

class FlowLayout(QGridLayout):
    def __init__(self, /, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.flow_num: int = 0
    
    def setFlowNum(self, num: int):
        self.flow_num = num
        self.flow()

    def flow(self):
        children = []
        for row in range(self.rowCount()):
            for col in range(self.columnCount()):
                item = self.itemAtPosition(row, col)
                if item is not None:
                    widget = item.widget()
                    if widget is not None:
                        self.removeWidget(widget)
                        children.append(widget)
        for child in children:
            self.addFlowWidget(child)

    def addFlowWidget(self, widget: QWidget):
        count = self.count()
        row = count // self.flow_num
        col = count % self.flow_num
        self.addWidget(widget, row, col)

class ChoiceWindow(QFrame, Ui_MessageBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._setUpUi(self)
        self.mainLayout.setContentsMargins(0,0,0,0)

        self.choiceLayout = QVBoxLayout()
        self.choiceLayout.setContentsMargins(15, 15, 15, 0)
        self.mainLayout.addLayout(self.choiceLayout)
        self._init_button()

        self.title = TitleLabel("选择导出内容")
        self.choiceLayout.addWidget(self.title, alignment=Qt.AlignmentFlag.AlignTop| Qt.AlignmentFlag.AlignLeft)
        self.choiceLayout.addSpacing(20)

        self.galaxyFrame = QFrame()
        self.set_border(self.galaxyFrame)
        self.starFrame = QFrame()
        self.set_border(self.starFrame)
        self.planetFrame = QFrame()
        self.set_border(self.planetFrame)
        self.galaxyLayout = QVBoxLayout(self.galaxyFrame)
        self.starLayout = QVBoxLayout(self.starFrame)
        self.planetLayout = QVBoxLayout(self.planetFrame)
        self.choiceLayout.addWidget(self.galaxyFrame)
        self.choiceLayout.addSpacing(10)
        self.choiceLayout.addWidget(self.starFrame)
        self.choiceLayout.addSpacing(10)
        self.choiceLayout.addWidget(self.planetFrame)
        self.__init__galaxy()
        self.__init__planet()
        self.__init__star()

    def __init__galaxy(self):
        self.galaxy_title = BodyLabel("选择导出的星系信息")
        self.galaxy_box = ConfigCheckBox("创建星系表", config_key="enable", config_obj=cfg.config.csv_galaxy)
        self.galaxyTitleLayout = QHBoxLayout()
        self.galaxyTitleLayout.setContentsMargins(0, 0, 0, 0)
        self.galaxyTitleLayout.addWidget(self.galaxy_title, alignment=Qt.AlignmentFlag.AlignLeft)
        self.galaxyTitleLayout.addWidget(self.galaxy_box, alignment=Qt.AlignmentFlag.AlignRight)
        self.galaxyGridLayout = FlowLayout()
        self.galaxyGridLayout.setFlowNum(4)
        self.galaxyGridLayout.setContentsMargins(0, 0, 0, 0)
        self.galaxy_veins_box = ConfigCheckBox("矿脉", config_key="veins", config_obj=cfg.config.csv_galaxy)
        self.galaxy_gas_box = ConfigCheckBox("气体矿脉", config_key="gas_veins", config_obj=cfg.config.csv_galaxy)
        self.galaxy_liquid_box = ConfigCheckBox("液体", config_key="liquid", config_obj=cfg.config.csv_galaxy)
        self.galaxy_stars_types_box = ConfigCheckBox("恒星类型", config_key="stars_types", config_obj=cfg.config.csv_galaxy)
        self.galaxyGridLayout.addFlowWidget(self.galaxy_veins_box)
        self.galaxyGridLayout.addFlowWidget(self.galaxy_gas_box)
        self.galaxyGridLayout.addFlowWidget(self.galaxy_liquid_box)
        self.galaxyGridLayout.addFlowWidget(self.galaxy_stars_types_box)


        self.galaxyLayout.addLayout(self.galaxyTitleLayout)
        self.galaxyLayout.addSpacing(10)
        self.galaxyLayout.addLayout(self.galaxyGridLayout)

    def __init__star(self):
        self.star_title = BodyLabel("选择导出的恒星信息")
        self.star_box = ConfigCheckBox("创建恒星表", config_key="enable", config_obj=cfg.config.csv_star)
        self.starTitleLayout = QHBoxLayout()
        self.starTitleLayout.setContentsMargins(0, 0, 0, 0)
        self.starTitleLayout.addWidget(self.star_title, alignment=Qt.AlignmentFlag.AlignLeft)
        self.starTitleLayout.addWidget(self.star_box, alignment=Qt.AlignmentFlag.AlignRight)

        self.star_veins_box = ConfigCheckBox("矿脉", config_key="veins", config_obj=cfg.config.csv_star)
        self.star_gas_box = ConfigCheckBox("气体矿脉", config_key="gas_veins", config_obj=cfg.config.csv_star)
        self.star_liquid_box = ConfigCheckBox("液体", config_key="liquid", config_obj=cfg.config.csv_star)
        self.star_star_types_box = ConfigCheckBox("恒星类型", config_key="star_type", config_obj=cfg.config.csv_star)
        self.star_distance_box = ConfigCheckBox("距离", config_key="distance", config_obj=cfg.config.csv_star)
        self.star_location_box = ConfigCheckBox("位置", config_key="location", config_obj=cfg.config.csv_star)
        self.star_lumino_box = ConfigCheckBox("戴森球亮度", config_key="ds_lumino", config_obj=cfg.config.csv_star)
        self.star_radius_box = ConfigCheckBox("戴森球半径", config_key="ds_radius", config_obj=cfg.config.csv_star)

        self.starGridLayout = FlowLayout()
        self.starGridLayout.setFlowNum(4)
        self.starGridLayout.setContentsMargins(0, 0, 0, 0)
        self.starGridLayout.addFlowWidget(self.star_veins_box)
        self.starGridLayout.addFlowWidget(self.star_gas_box)
        self.starGridLayout.addFlowWidget(self.star_liquid_box)
        self.starGridLayout.addFlowWidget(self.star_star_types_box)
        self.starGridLayout.addFlowWidget(self.star_distance_box)
        self.starGridLayout.addFlowWidget(self.star_location_box)
        self.starGridLayout.addFlowWidget(self.star_lumino_box)
        self.starGridLayout.addFlowWidget(self.star_radius_box)

        self.starLayout.addLayout(self.starTitleLayout)
        self.starLayout.addSpacing(10)
        self.starLayout.addLayout(self.starGridLayout)

    def __init__planet(self):
        self.planet_title = BodyLabel("选择导出的行星信息")
        self.planet_box = ConfigCheckBox("创建行星表", config_key="enable", config_obj=cfg.config.csv_planet)
        self.planetTitleLayout = QHBoxLayout()
        self.planetTitleLayout.setContentsMargins(0, 0, 0, 0)
        self.planetTitleLayout.addWidget(self.planet_title, alignment=Qt.AlignmentFlag.AlignLeft)
        self.planetTitleLayout.addWidget(self.planet_box, alignment=Qt.AlignmentFlag.AlignRight)

        self.planet_veins_box = ConfigCheckBox("矿脉", config_key="veins", config_obj=cfg.config.csv_planet)
        self.planet_gas_box = ConfigCheckBox("气体矿脉", config_key="gas_veins", config_obj=cfg.config.csv_planet)
        self.planet_liquid_box = ConfigCheckBox("液体", config_key="liquid", config_obj=cfg.config.csv_planet)
        self.planet_star_types_box = ConfigCheckBox("星球类型", config_key="planet_type", config_obj=cfg.config.csv_planet)
        self.planet_distance_box = ConfigCheckBox("距离", config_key="distance", config_obj=cfg.config.csv_planet)
        self.planet_location_box = ConfigCheckBox("位置", config_key="location", config_obj=cfg.config.csv_planet)
        self.planet_sing_box = ConfigCheckBox("特点", config_key="singularity", config_obj=cfg.config.csv_planet)
        self.planet_dsp_box = ConfigCheckBox("戴森球接收", config_key="dsp_level", config_obj=cfg.config.csv_planet)
        self.planet_wind_box = ConfigCheckBox("风能利用率", config_key="wind_usage", config_obj=cfg.config.csv_planet)
        self.planet_light_box = ConfigCheckBox("光能利用率", config_key="light_usage", config_obj=cfg.config.csv_planet)

        self.planetGridLayout = FlowLayout()
        self.planetGridLayout.setFlowNum(4)
        self.planetGridLayout.setContentsMargins(0, 0, 0, 0)
        self.planetGridLayout.addFlowWidget(self.planet_veins_box)
        self.planetGridLayout.addFlowWidget(self.planet_gas_box)
        self.planetGridLayout.addFlowWidget(self.planet_liquid_box)
        self.planetGridLayout.addFlowWidget(self.planet_star_types_box)
        self.planetGridLayout.addFlowWidget(self.planet_sing_box)
        self.planetGridLayout.addFlowWidget(self.planet_distance_box)
        self.planetGridLayout.addFlowWidget(self.planet_location_box)
        self.planetGridLayout.addFlowWidget(self.planet_dsp_box)
        self.planetGridLayout.addFlowWidget(self.planet_wind_box)
        self.planetGridLayout.addFlowWidget(self.planet_light_box)

        self.planetLayout.addLayout(self.planetTitleLayout)
        self.planetLayout.addSpacing(10)
        self.planetLayout.addLayout(self.planetGridLayout)
        

    def _init_button(self):
        self.buttonGroup = QFrame()
        self.buttonLayout = QHBoxLayout(self.buttonGroup)
        self.buttonLayout.setContentsMargins(15, 15, 15, 15)
        self.buttonLayout.setSpacing(10)
        self.mainLayout.addStretch()
        self.mainLayout.addWidget(self.buttonGroup)
        self.yesButton = PrimaryPushButton("导出")
        self.cancelButton = PushButton("取消")
        self.buttonLayout.addWidget(self.yesButton)
        self.buttonLayout.addWidget(self.cancelButton)
        qss = """
.QFrame {
    border-radius: 0px;
    border-bottom-left-radius: 8px;
    border-bottom-right-radius: 8px;
    background-color: --background-color;
}
"""
        self.buttonGroup.setStyleSheet(qss.replace("--background-color", "rgb(243, 243, 243)" if not isDarkTheme() else "rgb(32, 32, 32)"))

class PreviewWindow(QFrame, Ui_MessageBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._setUpUi(self)
        self._lock_resize = False

        self.title = BodyLabel("预览")
        self.mainLayout.addWidget(self.title)
    
    def resizeEvent(self, event: QResizeEvent) -> None:
        if self._lock_resize:
            return super().resizeEvent(event)
        self._lock_resize = True
        re = QResizeEvent(event)
        new_height = max(re.size().height() - 60, 0)
        self.resize(re.size().width(), new_height)
        self._lock_resize = False
        return super().resizeEvent(event)

class ExportWindow(MaskDialogBase):
    def __init__(self, parent):
        super().__init__(parent)
        self.setShadowEffect(60, (0, 10), QColor(0, 0, 0, 50))
        # self.widget.setGraphicsEffect(None)

        self.setMaskColor(QColor(0, 0, 0, 110))
        self.mainLayout = QHBoxLayout(self.widget)
        self.mainLayout.setContentsMargins(25, 25, 25, 25)
        self.mainLayout.setSpacing(15)
        
        self.widget.setMinimumHeight(400)
        self.widget.setMinimumWidth(600)
        qss = """
QFrame {
    border-radius: 8px;
    background-color: transparent;
}
            """
        self.setStyleSheet(qss)

        self.left_dialog = ChoiceWindow()
        self.right_dialog = PreviewWindow()
        self.mainLayout.addWidget(self.left_dialog, 3)
        self.mainLayout.addWidget(self.right_dialog, 2)

        self.left_dialog.yesButton.clicked.connect(self._onYesButtonClicked)
        self.left_dialog.cancelButton.clicked.connect(self._onCancelButtonClicked)

    def _onYesButtonClicked(self):
        self.accept()
    
    def _onCancelButtonClicked(self):
        self.reject()
    
    def _onDone(self, code):
        super()._onDone(code)
        self.deleteLater()

    def eventFilter(self, obj, e: QEvent):
        if obj is self.window():
            if e.type() == QEvent.Resize:
                self.resize(self.parent().size())
                return QDialog.eventFilter(self, obj, e)

        return super().eventFilter(obj, e)

if __name__ == "__main__":
    from PySide6.QtWidgets import QApplication
    import sys

    class window(QFrame):
        def __init__(self):
            super().__init__()
            self.setWindowTitle("测试")
            self.resize(800, 600)
            self.export_window = ExportWindow(self)
        def show_window(self):
            self.export_window.show()


    app = QApplication(sys.argv)
    a = window()
    a.show()
    a.show_window()
    sys.exit(app.exec())