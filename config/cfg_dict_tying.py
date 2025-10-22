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


class CelestialCondition(BaseModel):
    mediterranean: int = -1
    gas_giant: int = -1
    ice_giant: int = -1
    productive_gas_giant: int = -1
    arid_desert: int = -1
    ash_tundra: int = -1
    ocean_jungle: int = -1
    lava: int = -1
    ice_tundra: int = -1
    barren_desert: int = -1
    gobi: int = -1
    volcanic_ash: int = -1
    red_stone: int = -1
    grassland: int = -1
    water_world: int = -1
    black_salt_flat: int = -1
    cherry_blossom_forest: int = -1
    hurricane_stone_forest: int = -1
    crimson_ice_lake: int = -1
    tropical_savanna: int = -1
    orange_crystal_desert: int = -1
    frigid_tundra: int = -1
    pandora_swamp: int = -1

    @classmethod
    def get_field_mapping(cls) -> dict[str, str]:
        """返回中文名称到英文字段名的映射"""
        return {
            "地中海": "mediterranean",
            "气态巨星": "gas_giant",
            "冰巨星": "ice_giant",
            "高产气巨": "productive_gas_giant",
            "干旱荒漠": "arid_desert",
            "灰烬冻土": "ash_tundra",
            "海洋丛林": "ocean_jungle",
            "熔岩": "lava",
            "冰原冻土": "ice_tundra",
            "贫瘠荒漠": "barren_desert",
            "戈壁": "gobi",
            "火山灰": "volcanic_ash",
            "红石": "red_stone",
            "草原": "grassland",
            "水世界": "water_world",
            "黑石盐滩": "black_salt_flat",
            "樱林海": "cherry_blossom_forest",
            "飓风石林": "hurricane_stone_forest",
            "猩红冰湖": "crimson_ice_lake",
            "热带草原": "tropical_savanna",
            "橙晶荒漠": "orange_crystal_desert",
            "极寒冻土": "frigid_tundra",
            "潘多拉沼泽": "pandora_swamp",
        }


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
    celestial_condition: CelestialCondition = CelestialCondition()
    planet_conditions: list[PlanetCondition] = [PlanetCondition()]


class GalaxyCondition(BaseModel):
    custom_name: str = "银河系条件"
    checked: int = 2

    celestial_condition: CelestialCondition = CelestialCondition()
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
