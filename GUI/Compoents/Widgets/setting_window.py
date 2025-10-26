from PySide6.QtWidgets import QWidget, QVBoxLayout, QGridLayout, QHBoxLayout
from .line_edit import LabelWithLimitLineEdit, LimitLineEdit

from qfluentwidgets import PushButton, PopUpAniStackedWidget, TitleLabel, CaptionLabel
from config import cfg
from config.cfg_dict_tying import VeinsCondition, BaseModel, VeinsName


# from typing import TYPE_CHECKING
# if TYPE_CHECKING:
from GUI.Compoents.sort_tree import SettingsTreeLeave

class SettingsWindow(QWidget):
    def __init__(
        self,
        parent=None,
        items: list[str] = ["wa", "al", "aw", "la", "lw", "la"],
        context: SettingsTreeLeave | None = None,
    ):
        super().__init__(parent)

        self.context = context  # 预留给设置使用的上下文
        self.config_obj = None
        self.config_key = None
        self.item_dict = None

        if hasattr(self.context, "items"):
            self.item_dict: dict[str, str] = self.context.items

        if hasattr(self.context, "config_key"):
            self.config_key = self.context.config_key
        if hasattr(self.context, "config_obj"):
            self.config_obj = self.context.config_obj

        if self.item_dict is not None:
            items = list(self.item_dict.values())
        else:
            items = items

        self.mainLayout = QVBoxLayout(self)

        self.titleLabel = TitleLabel(context.settingsButton.objectName())

        self.mainLayout.addWidget(self.titleLabel)

        if self.context.description is not None:
            self.subtitleLabel = CaptionLabel(self.context.description)
            self.mainLayout.addWidget(self.subtitleLabel)

        self.settingsLayout = QGridLayout()

        self._addItems(items)

        self.mainLayout.addLayout(self.settingsLayout)

        self.mainLayout.addStretch()

        self.buttonLayout = QHBoxLayout()
        self.returnButton = PushButton("返回")
        self.returnButton.clicked.connect(self._return_button_clicked)
        self.buttonLayout.addWidget(self.returnButton)
        self.mainLayout.addLayout(self.buttonLayout)

        # self.saveButton = PushButton("保存")
        # self.cancelButton = PushButton("取消")
        # self.saveButton.clicked.connect(self._save_button_clicked__)
        # self.cancelButton.clicked.connect(self._cancel_button_clicked__)
        # self.buttonLayout.addWidget(self.saveButton)
        # self.buttonLayout.addWidget(self.cancelButton)
        # self.mainLayout.addLayout(self.buttonLayout)

    def _return_button_clicked(self):
        parent: PopUpAniStackedWidget = self.parent()  # type: ignore
        parent.setCurrentIndex(0)
        parent.removeWidget(self)
        self.deleteLater()

    def _addItems(self, items: list[str]):
        from math import sqrt

        count = len(items)
        itemsEachLine = int(sqrt(count)) + 1
        j = 0
        for i, item in enumerate(items):
            if i % itemsEachLine == 0 and i != 0:
                j += 1
            i = i % itemsEachLine

            name = VeinsName().model_dump()

            veinsKeyDict = dict(zip(name.values(), name.keys()))
            condition = self.config_obj.veins_condition


            line_edit = LabelWithLimitLineEdit(
                veinsKeyDict.get(item, item),
                condition,
                label=item,
                type_input="int",
                min_value=0,
                default_value=0
            )

            self.settingsLayout.addWidget(line_edit, j, i)

    # def _save_button_clicked__(self):
    #     parent: PopUpAniStackedWidget = self.parent()  # type: ignore
    #     error_flag = False
    #     for i in range(self.settingsLayout.count()):
    #         widget = self.settingsLayout.itemAt(i).widget()
    #         if isinstance(widget, LabelWithLimitLineEdit):
    #             key = widget.config_key
    #             value = widget.text().strip()
    #             try:
    #                 itext: int = int(value) if len(value) > 0 else 0
    #                 if self.config_obj and self.config_key:
    #                     veins_condition: VeinsCondition = getattr(
    #                         self.config_obj, self.config_key
    #                     )
    #                     setattr(veins_condition, key, itext)
    #             except Exception as e:
    #                 error_flag = True
    #                 print(f"设置配置项 {key} 失败: {e}")

    #     if error_flag is False:
    #         if self.config_obj:
    #             cfg.save()

    #     parent.setCurrentIndex(0)
    #     parent.removeWidget(self)
    #     self.deleteLater()

    # def _cancel_button_clicked__(self):
    #     parent: PopUpAniStackedWidget = self.parent()  # type: ignore

    #     parent.setCurrentIndex(0)
    #     parent.removeWidget(self)
    #     self.deleteLater()


if __name__ == "__main__":
    import sys

    pass
