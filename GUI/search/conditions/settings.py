from math import isfinite

from PySide6.QtWidgets import QGridLayout
from qfluentwidgets import MessageBoxBase, TitleLabel, CaptionLabel

from ...Compoents.Widgets.line_edit import LabelWithLimitLineEdit
from config import cfg
from language import tr, tr_domain
from .fields import SettingsTreeLeave


class SettingsWindow(MessageBoxBase):
    def __init__(self, parent, context: SettingsTreeLeave):
        super().__init__(parent)
        self.context = context
        self.condition = getattr(context.config_obj, context.config_key)
        self.line_edits = []

        self.yesButton.setText(tr("common.confirm"))
        self.cancelButton.setText(tr("common.cancel"))
        self.widget.setMinimumWidth(720)

        self.titleLabel = TitleLabel(context.display_title, self)
        self.viewLayout.addWidget(self.titleLabel)
        if context.description is not None:
            self.subtitleLabel = CaptionLabel(context.description, self)
            self.subtitleLabel.setWordWrap(True)
            self.viewLayout.addWidget(self.subtitleLabel)

        self.settingsLayout = QGridLayout()
        self.settingsLayout.setHorizontalSpacing(12)
        self.settingsLayout.setVerticalSpacing(12)
        self._addItems()
        self.viewLayout.addLayout(self.settingsLayout)

        self.warningLabel = CaptionLabel(self)
        if cfg.config.condition_mode == "advanced":
            self.warningLabel.setTextColor("#c42b1c", "#ff99a4")
            self.warningLabel.hide()
        self.warningLabel.setWordWrap(True)
        self.viewLayout.addWidget(self.warningLabel)

    def _addItems(self):
        is_veins_amount = self.context.config_key == "veins_amount_condition"
        for index, (veins_key, item) in enumerate(self.context.items.items()):
            type_input = "int"
            if is_veins_amount and veins_key == "oil":
                type_input = "liquid_float"
                display_item = tr("search.condition_tree.settings.oil_rate_label")
            else:
                display_item = tr_domain("veins", item)

            line_edit = LabelWithLimitLineEdit(
                veins_key,
                self.condition,
                label=display_item,
                type_input=type_input,
                min_value=0,
                default_value=0,
            )
            # Validate and save together on confirmation instead of on focus loss.
            line_edit.editingFinished.disconnect(line_edit._on_text_edited)
            line_edit.setMinimumWidth(200)
            self.line_edits.append(line_edit)
            self.settingsLayout.addWidget(line_edit, index // 3, index % 3)

        for column in range(3):
            self.settingsLayout.setColumnStretch(column, 1)

    def validate(self):
        values = {}
        for line_edit in self.line_edits:
            text = line_edit.text().strip()
            value = line_edit._type_convert(text) if text else 0
            error = None
            if value is None or (isinstance(value, float) and not isfinite(value)):
                error = tr("common.invalid_number")
            elif value < 0:
                error = tr("common.min_value").format(value=0)
            if error is not None:
                self.warningLabel.setText(f"{line_edit.label_box.text()}: {error}")
                self.warningLabel.show()
                line_edit.setFocus()
                line_edit.selectAll()
                return False
            if line_edit.type_input == "liquid_float":
                value = round(round(value, 2) * 25000)
            values[line_edit.config_key] = value

        for key, value in values.items():
            setattr(self.condition, key, value)
        cfg.save()
        self.context.edited = any(value != 0 for value in values.values())
        return True
