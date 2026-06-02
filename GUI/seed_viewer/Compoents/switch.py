from qfluentwidgets import SwitchButton
from language import tr

class SortOrderSwitch(SwitchButton):
    def __init__(self):
        super().__init__()
        self.setChecked(False)
        self.setOffText(tr("viewer.switch.descending"))
        self.setOnText(tr("viewer.switch.ascending"))

class QuickSortSwitch(SwitchButton):
    def __init__(self):
        super().__init__()
        self.setChecked(True)
        self.setOffText(tr("viewer.switch.standard"))
        self.setOnText(tr("viewer.switch.quick"))
