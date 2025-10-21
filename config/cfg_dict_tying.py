from pydantic import BaseModel, Field

class VeinsCondition(BaseModel):
    iron: int = 0
    copper: int = 0
    silicon: int = 0
    titanium: int = 0
    stone: int = 0
    coal: int = 0
    oil: int = 0
    ice: int = 0
    kimberley: int = 0
    silicon: int = 0
    organic_crystal: int = 0
    grating_stone: int = 0
    bamboo_shoot_crystal: int = 0
    monopolar_magnet: int = 0

class PlanetCondition(BaseModel):
    custom_name: str = "行星条件"
    veins_condition: VeinsCondition = VeinsCondition()
    planet_type: str = "无 / 任意"
    singularity: str = "无 / 任意"
    liquid_type: str = "无 / 任意"

    full_coverd_dsp: bool = False
    fc_hited_planet_num: int = 0


class StarSystemCondition(BaseModel):
    custom_name: str = "恒星系条件"
    veins_condition: VeinsCondition = VeinsCondition()
    star_type: str = "无 / 任意"
    lumino_level: float = 0.0


    distance_level: float = 0.0
    distance_hited_star_num: int = 0

    planet_conditions: list[PlanetCondition] = [PlanetCondition()]

class GalaxyCondition(BaseModel):
    custom_name: str = "银河系条件"
    veins_condition: VeinsCondition = VeinsCondition()
    star_system_conditions: list[StarSystemCondition] = [StarSystemCondition()]

class ConditionConfig(BaseModel):
    custom_name: str = "条件"
    galaxy_condition: GalaxyCondition = GalaxyCondition()

class GUIConfig(BaseModel):
    seed_range: tuple[int, int] = (0, 99999999)
    step_size: int = 1
    star_num_range: tuple[int, int] = (32, 64)
    max_thread: int = 4
    batch_size: int = 1000
    record_seed: bool = True
    conditions: list[ConditionConfig] = [ConditionConfig()]


if __name__ == "__main__":
    cfg = GUIConfig()

    print(cfg)