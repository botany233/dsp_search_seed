from pydantic import BaseModel, Field
from uuid import UUID, uuid4
from multiprocessing import cpu_count

class VeinsName(BaseModel):
    iron: str = "铁"
    copper: str = "铜"
    silicon: str = "硅"
    titanium: str = "钛"
    stone: str = "石"
    coal: str = "煤"
    oil: str = "油"
    ice: str = "可燃冰"
    kimberley: str = "金伯利"
    fractal_silicon: str = "分型硅"
    organic_crystal: str = "有机晶体"
    grating_stone: str = "光栅石"
    bamboo_shoot_crystal: str = "刺笋结晶"
    monopolar_magnet: str = "单极磁石"

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
    fractal_silicon: int = 0
    organic_crystal: int = 0
    grating_stone: int = 0
    bamboo_shoot_crystal: int = 0
    monopolar_magnet: int = 0

class PlanetCondition(BaseModel):
    custom_name: str = "星球条件"
    checked: bool = True

    planet_type: str = "无限制"
    singularity: str = "无限制"
    liquid_type: str = "无限制"

    dsp_level: str = "无限制"
    satisfy_num: int = 1

    veins_condition: VeinsCondition = VeinsCondition()
    veins_point_condition: VeinsCondition = VeinsCondition()
    uuid: UUID = Field(default_factory=uuid4)

class StarCondition(BaseModel):
    custom_name: str = "恒星系条件"
    checked: bool = True

    star_type: str = "无限制"
    lumino_level: float = 0.0
    distance_level: float = -1.0
    satisfy_num: int = 1

    veins_condition: VeinsCondition = VeinsCondition()
    veins_point_condition: VeinsCondition = VeinsCondition()
    planet_condition: list[PlanetCondition] = []
    uuid: UUID = Field(default_factory=uuid4)

class GalaxyCondition(BaseModel):
    custom_name: str = "星系条件"
    checked: bool = True

    veins_condition: VeinsCondition = VeinsCondition()
    veins_point_condition: VeinsCondition = VeinsCondition()
    star_condition: list[StarCondition] = []
    planet_condition: list[PlanetCondition] = []

class GUIConfig(BaseModel):
    start_seed: int = 0
    end_seed: int = 99999
    start_star_num: int = 32
    end_star_num: int = 64
    max_thread: int = cpu_count()
    batch_size: int = 128
    ui_scale_factor: float = 1.0
    galaxy_condition: GalaxyCondition = GalaxyCondition()
    save_name: str = "seed"
    search_mode: int = 0  # 0: 范围搜索, 1: 二次搜索

if __name__ == "__main__":
    # cfg = GUIConfig()
    # print(cfg)

    aaa = VeinsCondition().model_dump()
    print(aaa)

    bbb = VeinsName().model_dump()
    print(bbb)
