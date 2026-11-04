from CApi import GalaxyDataSimple, planet_types_c

def handle_planet_sort(subtype: str):
    return GetPlanetValue(subtype)

class GetPlanetValue:
    gas_id = planet_types_c.index("气态巨星")
    high_gas_id = planet_types_c.index("高产气巨")
    def __init__(self, n):
        self.n = planet_types_c.index(n)

    def __call__(self, galaxy_data: GalaxyDataSimple) -> int:
        if self.n == self.gas_id:
            return galaxy_data.planet_type_num[self.gas_id] + galaxy_data.planet_type_num[self.high_gas_id]
        else:
            return galaxy_data.planet_type_num[self.n]
