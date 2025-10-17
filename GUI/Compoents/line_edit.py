from qfluentwidgets import LineEdit, BodyLabel


__all__ = ["LabelWithLineEdit"]

class LabelWithLineEdit(LineEdit):
    def __init__(self, label: str = "You should give me even a foo as least", parent=None):
        super().__init__(parent)
        self.label_box = BodyLabel(label)
        self.label_box.setMinimumWidth(20)
        self.hBoxLayout.insertWidget(0, self.label_box)