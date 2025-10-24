from pydantic import BaseModel, Field
from uuid import UUID, uuid4
from multiprocessing import cpu_count

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
    custom_name: str = "星球条件"
    checked: bool = True

    planet_type: str = "无限制"
    singularity: str = "无限制"
    liquid_type: str = "无限制"

    full_coverd_dsp: bool = False
    fc_hited_planet_num: int = -1

    veins_condition: VeinsCondition = VeinsCondition()
    uuid: UUID = Field(default_factory=uuid4)

class StarCondition(BaseModel):
    custom_name: str = "恒星系条件"
    checked: bool = True

    star_type: str = "无限制"
    lumino_level: float = -1.0
    distance_level: float = -1.0
    distance_hited_star_num: int = -1

    veins_condition: VeinsCondition = VeinsCondition()
    planet_condition: list[PlanetCondition] = []
    uuid: UUID = Field(default_factory=uuid4)

class GalaxyCondition(BaseModel):
    custom_name: str = "星系条件"
    checked: bool = True

    veins_condition: VeinsCondition = VeinsCondition()
    star_condition: list[StarCondition] = []
    planet_condition: list[PlanetCondition] = []

class GUIConfig(BaseModel):
    seed_range: tuple[int, int] = (0, 99999)
    star_num_range: tuple[int, int] = (32, 64)
    max_thread: int = cpu_count()
    batch_size: int = 32
    ui_scale_factor: float = 1.0
    galaxy_condition: GalaxyCondition = GalaxyCondition()
    save_name: str = "seed"

if __name__ == "__main__":
    # cfg = GUIConfig()
    # print(cfg)

    aaa = VeinsCondition().model_dump()
    print(aaa)

    bbb = VeinsName().model_dump()
    print(bbb)