from enum import Enum
from qfluentwidgets import FluentIconBase
from qfluentwidgets.common.config import Theme

class DSPIcons(FluentIconBase, Enum):
    BAMBOO = "bamboo"
    COAL = "coal"
    COPPER = "copper"
    CASIMIR = "casimir"
    DEUTERIUM = "deuterium"
    DIAMOND = "diamond"
    FIREICE = "fireice"
    GRATING = "grating"
    HYDROGEN = "hydrogen"
    IRON = "iron"
    MAG = "mag"
    OIL = "oil"
    RUBBER = "rubber"
    SILICIUM = "silicium"
    STONE = "stone"
    SULPHURIC_ACID = "sulphuric_acid"
    TITANIUM = "titanium"
    WATER = "water"

    def path(self, theme=Theme.AUTO):
        return f'./assets/veins/{self.value}.png'
