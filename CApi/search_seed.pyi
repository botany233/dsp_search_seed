class PlanetData:
    name: str
    type: str
    type_id: int
    singularity: int
    singularity_str: list[str]
    seed: int
    lumino: float
    wind: float
    radius: float
    liquid: int
    is_gas: bool
    dsp_level: int
    veins_group: list[int]
    veins_point: list[int]
    gas_veins: list[float]

class StarData:
    name: str
    type: str
    type_id: int
    seed: int
    distance: float
    dyson_lumino: float
    dyson_radius: float
    planets: list[PlanetData]
    veins_group: list[int]
    veins_point: list[int]
    gas_veins: list[float]
    liquid: list[int]

class GalaxyData:
    seed: int
    star_num: int
    stars: list[StarData]
    veins_group: list[int]
    veins_point: list[int]
    gas_veins: list[float]
    liquid: list[int]

class PlanetCondition:
    satisfy_num: int = 1
    dsp_level: int = 0
    type: int = 0
    liquid: int = 0
    singularity: int = 0
    need_veins: int = 0
    veins_group: list[int] = [0] * 14
    veins_point: list[int] = [0] * 14

class StarCondition:
    satisfy_num: int = 1
    type: int = 0
    distance: float = 1000.0
    dyson_lumino: float = 0.0
    need_veins: int = 0
    veins_group: list[int] = [0] * 14
    veins_point: list[int] = [0] * 14
    planets: list[PlanetCondition] = []

class GalaxyCondition:
    need_veins: int = 0
    veins_group: list[int] = [0] * 14
    veins_point: list[int] = [0] * 14
    stars: list[StarCondition] = []
    planets: list[PlanetCondition] = []

def get_current_device_id_c() -> int: ...
def set_device_id_c(device_id:int) -> None: ...
def get_device_info_c() -> list[str]: ...
def galaxy_condition_to_struct(galaxy_condition:dict) -> GalaxyCondition: ...
def get_galaxy_data_c(seed:int, star_num:int, quick:bool) -> GalaxyData: ...
def check_batch_c(start_seed:int, end_seed:int, start_star_num:int, end_star_num:int, galaxy_condition:dict, quick: bool) -> list: ...
def check_precise_c(seed_list:list[int], star_num_list:list[int], galaxy_condition: dict, quick: bool) -> list: ...
