from CApi import GalaxyData

vein_names = ["铁", "铜", "硅", "钛", "石", "煤", "油", "可燃冰", "金伯利",
              "分型硅", "有机晶体", "光栅石", "刺笋结晶", "单极磁石"]

def handle_veins_sort(subtype: str):
    n = vein_names.index(subtype)
    return GetVeinsValue(n)

class GetVeinsValue():
    def __init__(self, n):
        self.n = n

    def __call__(self, galaxy_data: GalaxyData) -> int:
        return galaxy_data.veins[self.n]
