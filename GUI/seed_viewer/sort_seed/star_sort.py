from CApi import GalaxyDataSimple, star_types_c

def handle_star_sort(subtype: str):
    return GetStarValue(subtype)

class GetStarValue:
    def __init__(self, n):
        self.n = star_types_c.index(n)

    def __call__(self, galaxy_data: GalaxyDataSimple) -> int:
        return galaxy_data.star_type_num[self.n]
