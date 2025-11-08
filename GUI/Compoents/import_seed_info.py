from qfluentwidgets import BodyLabel
from .seed_manager import SeedManager

class ImportSeedInfo(BodyLabel):
    def __init__(self, seed_manager: SeedManager):
        super().__init__()
        self.seed_manager = seed_manager
        self.fresh()

    def fresh(self) -> None:
        count = self.seed_manager.get_seeds_count()
        self.setText(f"种子数：{count}")
