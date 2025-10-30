from CApi import GalaxyData, planet_types_c

def handle_planet_sort(subtype: str):
    n = planet_types_c.index(subtype)
    return GetPlanetValue(n)

class GetPlanetValue():
    def __init__(self, n):
        self.n = n

    def __call__(self, galaxy_data: GalaxyData) -> int:
        if self.n == 22:
            return galaxy_data.planet_type_nums[21] + galaxy_data.planet_type_nums[22]
        else:
            return galaxy_data.planet_type_nums[self.n]
