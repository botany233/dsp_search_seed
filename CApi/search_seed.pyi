class PlanetData:
    name: str
    type: str
    singularity: list[str]
    seed: int
    lumino: float
    wind: float
    radius: float
    liquid: int
    is_gas: bool
    is_in_dsp: bool
    is_on_dsp: bool
    veins: list[int]
    veins_point: list[int]
    gas_veins: list[float]

class StarData:
    name: str
    type: str
    seed: int
    distance: float
    dyson_lumino: float
    dyson_radius: float
    planets: list[PlanetData]
    veins: list[int]
    gas_veins: list[float]
    liquid: list[int]

class GalaxyData:
    seed: int
    star_num: int
    stars: list[StarData]
    veins: list[int]
    gas_veins: list[float]
    planet_type_nums: list[int]
    star_type_nums: list[int]
    liquid: list[int]

def get_galaxy_data_c(seed:int, star_num:int) -> GalaxyData: ...
def check_batch_c(start_seed:int, end_seed:int, start_star_num:int, end_star_num:int, galaxy_condition:str, galaxy_condition_simple:str) -> list: ...
def check_precise_c(seed_list:list[int], star_num_list:list[int], galaxy_condition:str, galaxy_condition_simple:str) -> list: ...