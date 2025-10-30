from CApi import GalaxyData, star_types_c

def handle_star_sort(subtype: str):
    n = star_types_c.index(subtype)
    return GetStarValue(n)

class GetStarValue():
    def __init__(self, n):
        self.n = n

    def __call__(self, galaxy_data: GalaxyData) -> int:
        return galaxy_data.star_type_nums[self.n]
