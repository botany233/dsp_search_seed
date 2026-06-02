from qfluentwidgets import TitleLabel, CaptionLabel, LineEdit, MessageBoxBase
from language import tr

from .seed_list import SeedList

class ManualAddMessageBox(MessageBoxBase):
    def __init__(self, seed_list: SeedList, parent=None):
        super().__init__(parent)
        self.yesButton.setText(tr("common.confirm"))
        self.cancelButton.setText(tr("common.cancel"))

        title_label = TitleLabel(tr("viewer.manual_add.title"), self)
        self.viewLayout.addWidget(title_label)

        self.seed_id = LineEdit(self)
        self.seed_id.setPlaceholderText(tr("viewer.manual_add.seed_id"))
        self.viewLayout.addWidget(self.seed_id)
        self.star_num = LineEdit(self)
        self.star_num.setPlaceholderText(tr("viewer.manual_add.star_num"))
        self.viewLayout.addWidget(self.star_num)

        self.warning_label = CaptionLabel(self)
        self.viewLayout.addWidget(self.warning_label)
        self.buttonGroup.setMinimumWidth(370)

        self.seed_list = seed_list

    def validate(self):
        try:
            seed = int(self.seed_id.text())
            star_num = int(self.star_num.text())
        except Exception:
            self.warning_label.setText(tr("viewer.manual_add.invalid_seed"))
            return False
        else:
            if not (0 <= seed <= 99999999 and 32 <= star_num <= 64):
                self.warning_label.setText(tr("viewer.manual_add.invalid_seed"))
                return False
            elif self.seed_list.is_exist(seed, star_num):
                self.warning_label.setText(tr("viewer.manual_add.seed_exists"))
                return False
            else:
                return True
