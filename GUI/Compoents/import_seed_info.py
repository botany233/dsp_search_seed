from qfluentwidgets import BodyLabel
from CApi import SeedManager
from language import tr

class ImportSeedInfo(BodyLabel):
    def __init__(self, seed_manager: SeedManager):
        super().__init__()
        self.seed_manager = seed_manager
        self.fresh()

    def fresh(self) -> None:
        count = self.seed_manager.get_seeds_count()
        self.setText(tr("search.status.seed_count").format(count=count))
