from qfluentwidgets import TitleLabel, CaptionLabel, LineEdit, MessageBoxBase

class ManualAddMessageBox(MessageBoxBase):
    def __init__(self, seed_list: list, parent=None):
        super().__init__(parent)
        title_label = TitleLabel("手动添加种子", self)
        self.viewLayout.addWidget(title_label)

        self.seed_id = LineEdit(self)
        self.seed_id.setPlaceholderText("种子id")
        self.viewLayout.addWidget(self.seed_id)
        self.star_num = LineEdit(self)
        self.star_num.setPlaceholderText("恒星数")
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
            self.warning_label.setText("无效的种子！")
            return False
        else:
            if not (0 <= seed <= 99999999 and 32 <= star_num <= 64):
                self.warning_label.setText("无效的种子！")
                return False
            elif (seed, star_num) in [(i[0], i[1]) for i in self.seed_list]:
                self.warning_label.setText("种子已存在！")
                return False
            else:
                return True
