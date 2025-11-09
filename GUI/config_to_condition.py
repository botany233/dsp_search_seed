__all__ = ["config_to_galaxy_condition"]

from config.cfg_dict_tying import (
    GalaxyCondition,
    PlanetCondition,
    StarCondition,
    VeinsCondition,
    VeinsName,
)

def config_to_galaxy_condition(galaxy_cfg: GalaxyCondition) -> dict:
    galaxy_condition = {}
    if galaxy_cfg.checked:
        if (veins := get_veins_dict(galaxy_cfg.veins_condition)):
            galaxy_condition["veins"] = veins
        if (veins_point := get_veins_dict(galaxy_cfg.veins_point_condition)):
            galaxy_condition["veins_point"] = veins_point
    galaxy_condition["stars"] = [config_to_star_condition(star_cfg) for star_cfg in galaxy_cfg.star_condition]
    galaxy_condition["planets"] = [config_to_planet_condition(planet_cfg) for planet_cfg in galaxy_cfg.planet_condition]
    return galaxy_condition

def config_to_star_condition(star_cfg: StarCondition) -> dict:
    star_condition = {}
    if star_cfg.checked:
        if (veins := get_veins_dict(star_cfg.veins_condition)):
            star_condition["veins"] = veins
        if (veins_point := get_veins_dict(star_cfg.veins_point_condition)):
            star_condition["veins_point"] = veins_point
        if star_cfg.star_type != "无限制":
            star_condition["type"] = star_cfg.star_type
        if star_cfg.distance_level >= 0:
            star_condition["distance"] = star_cfg.distance_level
        if star_cfg.lumino_level > 0:
            star_condition["lumino"] = star_cfg.lumino_level
        if star_cfg.satisfy_num > 1:
            star_condition["satisfy_num"] = star_cfg.satisfy_num
    star_condition["planets"] = [config_to_planet_condition(planet_cfg) for planet_cfg in star_cfg.planet_condition]
    return star_condition

def config_to_planet_condition(planet_cfg: PlanetCondition) -> dict:
    planet_condition = {}
    if planet_cfg.checked:
        if (veins := get_veins_dict(planet_cfg.veins_condition)):
            planet_condition["veins"] = veins
        if (veins_point := get_veins_dict(planet_cfg.veins_point_condition)):
            planet_condition["veins_point"] = veins_point
        if planet_cfg.planet_type != "无限制":
            planet_condition["type"] = planet_cfg.planet_type
        if planet_cfg.singularity != "无限制":
            if planet_cfg.singularity == "永昼永夜":
                planet_condition["singularity"] = "潮汐锁定永昼永夜"
            else:
                planet_condition["singularity"] = planet_cfg.singularity
        if planet_cfg.liquid_type != "无限制":
            planet_condition["liquid"] = planet_cfg.liquid_type
        if planet_cfg.is_in_dsp:
            planet_condition["is_in_dsp"] = True
        if planet_cfg.is_on_dsp:
            planet_condition["is_on_dsp"] = True
        if planet_cfg.satisfy_num > 1:
            planet_condition["satisfy_num"] = planet_cfg.satisfy_num
    return planet_condition

def get_veins_dict(data: VeinsCondition) -> dict:
    veins = {}
    vein_names = VeinsName().model_dump()
    vein_data = data.model_dump()
    for key in vein_names.keys():
        if vein_data[key] > 0:
            veins[vein_names[key]] = vein_data[key]
    return veins
