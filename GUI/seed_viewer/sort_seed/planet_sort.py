from CApi import GalaxyData, planet_types_c

def handle_planet_sort(subtype: str):
    return GetPlanetValue(subtype)

class GetPlanetValue():
    def __init__(self, n):
        self.n = n

    def __call__(self, galaxy_data: GalaxyData) -> int:
        num = 0
        if self.n == "气态巨星":
            for star in galaxy_data.stars:
                for planet in star.planets:
                    if planet.type == "气态巨星" or planet.type == "高产气巨":
                        num += 1
        else:
            for star in galaxy_data.stars:
                for planet in star.planets:
                    if planet.type == self.n:
                        num += 1
        return num
