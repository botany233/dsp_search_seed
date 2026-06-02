from qfluentwidgets import CaptionLabel
from language import tr

from .seed_list import SeedList

class SeedText(CaptionLabel):
    def __init__(self, seed_list: SeedList):
        super().__init__()
        self.seed_list = seed_list
        self.fresh()

    def fresh(self):
        seed_num, max_seed = self.seed_list.get_seed_num()
        self.setText(tr("viewer.seed_text.count").format(seed_num=seed_num, max_seed=max_seed))
