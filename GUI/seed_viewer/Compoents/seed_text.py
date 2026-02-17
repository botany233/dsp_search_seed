from qfluentwidgets import CaptionLabel

from .seed_list import SeedList

class SeedText(CaptionLabel):
    def __init__(self, seed_list: SeedList):
        super().__init__()
        self.seed_list = seed_list
        self.fresh()

    def fresh(self):
        seed_num, max_seed = self.seed_list.get_seed_num()
        self.setText(f"种子数: {seed_num}/{max_seed}")
