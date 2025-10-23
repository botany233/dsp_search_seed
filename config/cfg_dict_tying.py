from pydantic import BaseModel, Field


class VeinsName(BaseModel):
    iron: str = "铁"
    copper: str = "铜"
    silicon: str = "硅"
    titanium: str = "钛"
    stone: str = "石头"
    coal: str = "煤炭"
    oil: str = "石油"
    ice: str = "可燃冰"
    kimberley: str = "金伯利"
    fractal_silicon: str = "分型硅"
    organic_crystal: str = "有机晶体"
    grating_stone: str = "光栅石"
    bamboo_shoot_crystal: str = "竹笋晶体"
    monopolar_magnet: str = "单极磁石"


class VeinsCondition(BaseModel):
    iron: int = -1
    copper: int = -1
    silicon: int = -1
    titanium: int = -1
    stone: int = -1
    coal: int = -1
    oil: int = -1
    ice: int = -1
    kimberley: int = -1
    fractal_silicon: int = -1
    organic_crystal: int = -1
    grating_stone: int = -1
    bamboo_shoot_crystal: int = -1
    monopolar_magnet: int = -1

class PlanetCondition(BaseModel):
    custom_name: str = "行星条件"
    checked: int = 2

    planet_type: str = "无 / 任意"
    singularity: str = "无 / 任意"
    liquid_type: str = "无 / 任意"

    full_coverd_dsp: bool = False
    fc_hited_planet_num: int = -1

    veins_condition: VeinsCondition = VeinsCondition()


class StarSystemCondition(BaseModel):
    custom_name: str = "恒星系条件"
    checked: int = 2

    star_type: str = "无 / 任意"
    lumino_level: float = -1.0
    distance_level: float = -1.0
    distance_hited_star_num: int = -1

    veins_condition: VeinsCondition = VeinsCondition()
    planet_conditions: list[PlanetCondition] = [PlanetCondition()]


class GalaxyCondition(BaseModel):
    custom_name: str = "银河系条件"
    checked: int = 2

    veins_condition: VeinsCondition = VeinsCondition()
    star_system_conditions: list[StarSystemCondition] = [StarSystemCondition()]


class GUIConfig(BaseModel):
    seed_range: tuple[int, int] = (0, 99999999)
    step_size: int = 1
    star_num_range: tuple[int, int] = (32, 64)
    max_thread: int = 4
    batch_size: int = 1000
    record_seed: bool = True
    ui_scale_factor: float = 1.0
    conditions: GalaxyCondition = GalaxyCondition()


if __name__ == "__main__":
    cfg = GUIConfig()

    print(cfg)
