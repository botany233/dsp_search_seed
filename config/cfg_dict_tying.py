from pydantic import BaseModel, Field
from typing import Annotated, Literal
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
    fractal_silicon: str = "分形硅"
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

    # planet_type: str = "无限制"
    planet_type: list[str] = Field(default_factory=list)
    # singularity: str = "无限制"
    singularity: list[str] = Field(default_factory=list)
    liquid_type: str = "无限制"

    dsp_level: str = "无限制"
    satisfy_num: int = 1

    veins_point_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    veins_amount_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    moon_conditions: list['PlanetCondition'] = Field(default_factory=list)
    uuid: UUID = Field(default_factory=uuid4)

class StarCondition(BaseModel):
    custom_name: str = "恒星系条件"
    checked: bool = True

    # star_type: str = "无限制"
    star_type: list[str] = Field(default_factory=list)
    lumino_level: float = 0.0
    distance_level: float = -1.0
    satisfy_num: int = 1

    veins_point_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    veins_amount_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    planet_condition: list[PlanetCondition] = Field(default_factory=list)
    uuid: UUID = Field(default_factory=uuid4)

class GalaxyCondition(BaseModel):
    custom_name: str = "星区条件"
    checked: bool = True
    valid_state: bool = True

    veins_point_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    veins_amount_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    star_condition: list[StarCondition] = Field(default_factory=list)
    planet_condition: list[PlanetCondition] = Field(default_factory=list)

# csv 导出设置
class GalaxyExportCondition(BaseModel):
    enable: bool = True
    veins_point: bool = True
    veins_amount: bool = False
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
    veins_point: bool = True
    veins_amount: bool = False
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
    dsp_degree: bool = False
    liquid: bool = True
    veins_point: bool = True
    veins_amount: bool = False
    gas_veins: bool = True
    planet_type: bool = True
    wind_usage: bool = True
    light_usage: bool = True

class CSVExportCondition(BaseModel):
    galaxy: GalaxyExportCondition = GalaxyExportCondition()
    star: StarExportCondition = StarExportCondition()
    planet: PlanetExportCondition = PlanetExportCondition()

def default_basic_condition() -> GalaxyCondition:
    return GalaxyCondition(
        star_condition=[StarCondition(planet_condition=[PlanetCondition(moon_conditions=[PlanetCondition()])])],
        planet_condition=[PlanetCondition(moon_conditions=[PlanetCondition()])],
    )


class SharedSearchConfig(BaseModel):
    start_seed: int = 0
    end_seed: int = 99999
    start_star_num: int = 32
    end_star_num: int = 64
    save_name: str = "seed"
    resource_rate: str = "无限"
    search_mode: Literal[0, 1] = 0  # 0: 范围搜索, 1: 二次搜索
    quick_check: bool = False


class BasicSearchConfig(BaseModel):
    galaxy_condition: GalaxyCondition = Field(default_factory=default_basic_condition)


class AdvancedEndpoint(BaseModel):
    kind: Literal["star", "planet"] = "star"
    star: StarCondition = Field(default_factory=StarCondition)
    planet: PlanetCondition = Field(default_factory=PlanetCondition)


class AdvancedStarNode(BaseModel):
    kind: Literal["star"] = "star"
    condition: StarCondition = Field(default_factory=StarCondition)


class AdvancedPlanetNode(BaseModel):
    kind: Literal["planet"] = "planet"
    condition: PlanetCondition = Field(default_factory=PlanetCondition)


class AdvancedMineralNode(BaseModel):
    kind: Literal["mineral"] = "mineral"
    checked: bool = True
    veins_point_condition: VeinsCondition = Field(default_factory=VeinsCondition)
    veins_amount_condition: VeinsCondition = Field(default_factory=VeinsCondition)


class AdvancedBondNode(BaseModel):
    kind: Literal["bond"] = "bond"
    checked: bool = True
    distance: float = Field(default=1000.0, ge=0, allow_inf_nan=False)
    satisfy_num: int = Field(default=1, ge=1, le=2147483647)
    con1: AdvancedEndpoint = Field(default_factory=AdvancedEndpoint)
    con2: AdvancedEndpoint = Field(default_factory=AdvancedEndpoint)


class AdvancedLogicNode(BaseModel):
    kind: Literal["logic"] = "logic"
    checked: bool = True
    operation: Literal["all", "any", "at_least", "not"] = "all"
    minimum: int = Field(default=1, ge=1, le=65535)
    children: list["AdvancedNode"] = Field(default_factory=list)


AdvancedNode = Annotated[
    AdvancedLogicNode | AdvancedStarNode | AdvancedPlanetNode | AdvancedMineralNode | AdvancedBondNode,
    Field(discriminator="kind"),
]
AdvancedLogicNode.model_rebuild()


class AdvancedSearchConfig(BaseModel):
    condition: AdvancedLogicNode = Field(default_factory=AdvancedLogicNode)


class GUIConfig(BaseModel):
    condition_mode: Literal["basic", "advanced"] = "basic"
    shared_search: SharedSearchConfig = Field(default_factory=SharedSearchConfig)
    basic_search: BasicSearchConfig = Field(default_factory=BasicSearchConfig)
    advanced_search: AdvancedSearchConfig = Field(default_factory=AdvancedSearchConfig)
    max_thread: int = cpu_count()
    ui_scale_factor: float = 1.0
    language: str = "zh_CN"
    local_size: int = 256
    device_name: str = "cpu"
    max_gpu_worker: int = 4
    use_gpu: bool = False # 仅仅是方便初始化, 不过确实是关联的
    name_language: str = "English"
    # csv
    csv: CSVExportCondition = CSVExportCondition()

    @property
    def active_search(self) -> BasicSearchConfig | AdvancedSearchConfig:
        return self.basic_search if self.condition_mode == "basic" else self.advanced_search

if __name__ == "__main__":
    # cfg = GUIConfig()
    # print(cfg)

    aaa = VeinsCondition().model_dump()
    print(aaa)

    bbb = VeinsName().model_dump()
    print(bbb)
