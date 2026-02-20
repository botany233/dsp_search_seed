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

class MoonCondition(BaseModel):
    custom_name: str = "卫星条件"
    checked: bool = True

    planet_type: str = "无限制"
    singularity: str = "无限制"
    liquid_type: str = "无限制"

    dsp_level: str = "无限制"
    satisfy_num: int = 1

    veins_condition: VeinsCondition = VeinsCondition()
    veins_point_condition: VeinsCondition = VeinsCondition()
    uuid: UUID = Field(default_factory=uuid4)

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
    moon_conditions: list[MoonCondition] = []
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

# csv 导出设置
class GalaxyExportCondition(BaseModel):
    enable: bool = True
    veins: bool = True
    gas_veins: bool = True
    liquid: bool = True
    star_types: bool = True
    planet_types: bool = True

class StarExportCondition(BaseModel):
    enable: bool = True
    distance: bool = True
    location: bool = False
    ds_radius: bool = True
    ds_lumino: bool = True
    veins: bool = True
    gas_veins: bool = True
    liquid: bool = True
    type: bool = True

class PlanetExportCondition(BaseModel):
    enable: bool = True
    star_name: bool = True
    star_lumino: bool = True
    star_distance: bool = True
    star_location: bool = False
    star_type: bool = True
    singularity: bool = True
    dsp_level: bool = True
    liquid: bool = True
    veins: bool = True
    gas_veins: bool = True
    planet_type: bool = True
    wind_usage: bool = True
    light_usage: bool = True

class CSVExportCondition(BaseModel):
    galaxy: GalaxyExportCondition = GalaxyExportCondition()
    star: StarExportCondition = StarExportCondition()
    planet: PlanetExportCondition = PlanetExportCondition()

class GUIConfig(BaseModel):
    start_seed: int = 0
    end_seed: int = 99999
    start_star_num: int = 32
    end_star_num: int = 64
    max_thread: int = cpu_count()
    ui_scale_factor: float = 1.0
    galaxy_condition: GalaxyCondition = GalaxyCondition()
    save_name: str = "seed"
    search_mode: int = 0  # 0: 范围搜索, 1: 二次搜索
    quick_check: bool = False
    local_size: int = 256
    device_name: str = "cpu"
    use_gpu: bool = False # 仅仅是方便初始化, 不过确实是关联的
    # csv
    csv: CSVExportCondition = CSVExportCondition()

if __name__ == "__main__":
    # cfg = GUIConfig()
    # print(cfg)

    aaa = VeinsCondition().model_dump()
    print(aaa)

    bbb = VeinsName().model_dump()
    print(bbb)
