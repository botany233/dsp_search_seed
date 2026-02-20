from copy import deepcopy
from os.path import join as j_

from PySide6.QtCore import QEvent, Qt
from PySide6.QtGui import QColor, QResizeEvent
from PySide6.QtWidgets import QHBoxLayout, QFrame, QVBoxLayout, QDialog, QGridLayout, QWidget, QFileDialog, QApplication
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
from CApi import GetDataManager
from config import cfg

from .save_seed_info import save_seed_info

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

# class FlowLayout(QGridLayout):
#     def __init__(self, /, parent: QWidget | None = None) -> None:
#         super().__init__(parent)
#         self.flow_num: int = 0
    
#     def setFlowNum(self, num: int):
#         self.flow_num = num
#         self.flow()

#     def flow(self):
#         children = []
#         for row in range(self.rowCount()):
#             for col in range(self.columnCount()):
#                 item = self.itemAtPosition(row, col)
#                 if item is not None:
#                     widget = item.widget()
#                     if widget is not None:
#                         self.removeWidget(widget)
#                         children.append(widget)
#         for child in children:
#             self.addFlowWidget(child)

#     def addFlowWidget(self, widget: QWidget):
#         count = self.count()
#         row = count // self.flow_num
#         col = count % self.flow_num
#         self.addWidget(widget, row, col)

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
        curTitleLayout = QHBoxLayout()
        curTitleLayout.setContentsMargins(0, 0, 0, 0)
        curTitleLayout.addWidget(BodyLabel("选择导出的星系信息"), alignment=Qt.AlignmentFlag.AlignLeft)
        curTitleLayout.addWidget(ConfigCheckBox("创建星系表", config_key="enable", config_obj=cfg.config.csv.galaxy), alignment=Qt.AlignmentFlag.AlignRight)

        boxs_paras = [
            [("恒星类型", "star_types"), ("行星类型", "planet_types")],
            [("液体", "liquid"), ("气体矿脉", "gas_veins"), ("矿脉", "veins")]
        ]

        curGridLayout = QGridLayout()
        curGridLayout.setContentsMargins(0, 0, 0, 0)
        for row, box_paras in enumerate(boxs_paras):
            for col, (display_text, config_key) in enumerate(box_paras):
                box = ConfigCheckBox(display_text, config_key=config_key, config_obj=cfg.config.csv.galaxy)
                curGridLayout.addWidget(box, row, col)

        self.galaxyLayout.addLayout(curTitleLayout)
        self.galaxyLayout.addSpacing(10)
        self.galaxyLayout.addLayout(curGridLayout)

    def __init__star(self):
        curTitleLayout = QHBoxLayout()
        curTitleLayout.setContentsMargins(0, 0, 0, 0)
        curTitleLayout.addWidget(BodyLabel("选择导出的恒星信息"), alignment=Qt.AlignmentFlag.AlignLeft)
        curTitleLayout.addWidget(ConfigCheckBox("创建恒星表", config_key="enable", config_obj=cfg.config.csv.star), alignment=Qt.AlignmentFlag.AlignRight)

        boxs_paras = [
            [("类型", "type"), ("距离", "distance"), ("坐标", "location"), ("液体", "liquid")],
            [("亮度", "ds_lumino"), ("戴森球半径", "ds_radius"), ("气体矿脉", "gas_veins"), ("矿脉", "veins")]
        ]

        curGridLayout = QGridLayout()
        curGridLayout.setContentsMargins(0, 0, 0, 0)
        for row, box_paras in enumerate(boxs_paras):
            for col, (display_text, config_key) in enumerate(box_paras):
                box = ConfigCheckBox(display_text, config_key=config_key, config_obj=cfg.config.csv.star)
                curGridLayout.addWidget(box, row, col)

        self.starLayout.addLayout(curTitleLayout)
        self.starLayout.addSpacing(10)
        self.starLayout.addLayout(curGridLayout)

    def __init__planet(self):
        curTitleLayout = QHBoxLayout()
        curTitleLayout.setContentsMargins(0, 0, 0, 0)
        curTitleLayout.addWidget(BodyLabel("选择导出的行星信息"), alignment=Qt.AlignmentFlag.AlignLeft)
        curTitleLayout.addWidget(ConfigCheckBox("创建行星表", config_key="enable", config_obj=cfg.config.csv.star), alignment=Qt.AlignmentFlag.AlignRight)

        boxs_paras = [
            [("恒星名称", "star_name"), ("恒星类型", "star_type"), ("恒星光度", "star_lumino"), ("恒星距离", "star_distance"), ("恒星坐标", "star_location")],
            [("星球类型", "planet_type"), ("词条", "singularity"), ("戴森球接收", "dsp_level"), ("液体", "liquid")],
            [("风能利用率", "wind_usage"), ("光能利用率", "light_usage"), ("气体矿脉", "gas_veins"), ("矿脉", "veins")]
        ]

        curGridLayout = QGridLayout()
        curGridLayout.setContentsMargins(0, 0, 0, 0)
        for row, box_paras in enumerate(boxs_paras):
            for col, (display_text, config_key) in enumerate(box_paras):
                box = ConfigCheckBox(display_text, config_key=config_key, config_obj=cfg.config.csv.planet)
                curGridLayout.addWidget(box, row, col)

        self.planetLayout.addLayout(curTitleLayout)
        self.planetLayout.addSpacing(10)
        self.planetLayout.addLayout(curGridLayout)

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
    def __init__(self, parent, selected_seeds):
        super().__init__(parent)
        self.selected_seeds = selected_seeds
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

    def __do_export_seeds(self) -> None:
        csv_config = deepcopy(cfg.config.csv)

        dir_path = QFileDialog.getExistingDirectory(
            self,
            "选择保存文件夹",
            # "export_seed"
        )

        if not dir_path:
            return

        get_data_manager = GetDataManager(cfg.config.max_thread, False, min(32, cfg.config.max_thread))
        for seed_id, star_num in self.selected_seeds:
            get_data_manager.add_task(seed_id, star_num)

        finish_num, task_num = 0, len(self.selected_seeds)
        while finish_num < task_num:
            results = get_data_manager.get_results()
            for result in results:
                file_path = j_(dir_path, f"{result.seed}_{result.star_num}.csv")
                save_seed_info(file_path, result, csv_config)
            finish_num += len(results)
            QApplication.processEvents()

    def _onYesButtonClicked(self):
        self.__do_export_seeds()
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