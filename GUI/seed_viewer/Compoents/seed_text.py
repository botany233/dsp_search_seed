from qfluentwidgets import CaptionLabel

class SeedText(CaptionLabel):
    def __init__(self, seed_list: list):
        super().__init__()
        self.seed_list = seed_list
        self.fresh()

    def fresh(self):
        self.setText(f"种子数: {len(self.seed_list)}")
