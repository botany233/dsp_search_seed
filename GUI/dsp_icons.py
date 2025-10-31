from enum import Enum
from qfluentwidgets import FluentIconBase
from qfluentwidgets.common.config import Theme

class DSPIcons(FluentIconBase, Enum):
    BAMBOO = "bamboo"
    COAL = "coal"
    COPPER = "copper"
    DEUTERIUM = "deuterium"
    DIAMOND = "diamond"
    FIREICE = "fireice"
    FRACTAL = "fractal"
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
    UNKNOWN = "unknown"

    def path(self, theme=Theme.AUTO):
        return f'./assets/veins/{self.value}.png'
    
    @staticmethod
    def match_icon(name: str) -> "DSPIcons":
        map_dict = {
            "铁": DSPIcons.IRON,
            "铜": DSPIcons.COPPER,
            "石": DSPIcons.STONE,
            "煤": DSPIcons.COAL,
            "油": DSPIcons.OIL,
            "水": DSPIcons.WATER,
            "钛": DSPIcons.TITANIUM,
            "硅": DSPIcons.SILICIUM,
            "分型硅": DSPIcons.FRACTAL,
            "氢": DSPIcons.HYDROGEN,
            "重氢": DSPIcons.DEUTERIUM,
            "可燃冰": DSPIcons.FIREICE,
            "金伯利": DSPIcons.DIAMOND,
            "单极磁石": DSPIcons.MAG,
            "光栅石": DSPIcons.GRATING,
            "刺笋结晶": DSPIcons.BAMBOO,
            "有机晶体": DSPIcons.RUBBER,
            "硫酸": DSPIcons.SULPHURIC_ACID,
        }
        return map_dict.get(name, DSPIcons.UNKNOWN)
