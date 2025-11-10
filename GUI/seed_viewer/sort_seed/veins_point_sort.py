from CApi import GalaxyData, vein_names_c

def handle_veins_point_sort(subtype: str):
    n = vein_names_c.index(subtype)
    return GetVeinsPointValue(n)

class GetVeinsPointValue():
    def __init__(self, n):
        self.n = n

    def __call__(self, galaxy_data: GalaxyData) -> int:
        return galaxy_data.veins_point[self.n]
