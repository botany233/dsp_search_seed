from CApi import GalaxyData, star_types_c

def handle_star_sort(subtype: str):
    return GetStarValue(subtype)

class GetStarValue():
    def __init__(self, n):
        self.n = n

    def __call__(self, galaxy_data: GalaxyData) -> int:
        num = 0
        for star in galaxy_data.stars:
            if star.type == self.n:
                num += 1
        return num
