from qfluentwidgets import SwitchButton

class SortOrderSwitch(SwitchButton):
    def __init__(self):
        super().__init__()
        self.setChecked(False)
        self.setOffText("降序排列")
        self.setOnText("升序排列")
