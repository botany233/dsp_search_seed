from qfluentwidgets import SwitchButton

class SortOrderSwitch(SwitchButton):
    def __init__(self):
        super().__init__()
        self.setChecked(False)
        self.setOffText("降序排列")
        self.setOnText("升序排列")

class QuickSortSwitch(SwitchButton):
    def __init__(self):
        super().__init__()
        self.setChecked(True)
        self.setOffText("标准模式")
        self.setOnText("快速模式")
