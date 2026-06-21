__all__ = ["config_to_galaxy_condition"]

from config.cfg_dict_tying import (
    GalaxyCondition,
    BondCondition,
    PlanetCondition,
    StarCondition,
    VeinsCondition,
    VeinsName,
)

def config_to_galaxy_condition(galaxy_cfg: GalaxyCondition) -> dict:
    galaxy_condition = {}
    if galaxy_cfg.checked:
        if (veins_point := get_veins_dict(galaxy_cfg.veins_point_condition)):
            galaxy_condition["veins_point"] = veins_point
        if (veins_amount := get_veins_dict(galaxy_cfg.veins_amount_condition)):
            galaxy_condition["veins_amount"] = veins_amount
    galaxy_condition["bonds"] = [config_to_bond_condition(bond_cfg) for bond_cfg in galaxy_cfg.bond_condition]
    galaxy_condition["stars"] = [config_to_star_condition(star_cfg) for star_cfg in galaxy_cfg.star_condition]
    galaxy_condition["planets"] = [config_to_planet_condition(planet_cfg) for planet_cfg in galaxy_cfg.planet_condition]
    return galaxy_condition

def config_to_bond_condition(bond_cfg: BondCondition) -> dict:
    bond_condition = {}
    if bond_cfg.checked:
        if bond_cfg.con1_is_planet:
            bond_condition["con1"] = config_to_planet_condition(bond_cfg.con1_planet)
        else:
            bond_condition["con1"] = config_to_star_condition(bond_cfg.con1_star)

        if bond_cfg.con2_is_planet:
            bond_condition["con2"] = config_to_planet_condition(bond_cfg.con2_planet)
        else:
            bond_condition["con2"] = config_to_star_condition(bond_cfg.con2_star)

        if bond_cfg.distance >= 0:
            bond_condition["distance"] = bond_cfg.distance
        if bond_cfg.satisfy_num > 1:
            bond_condition["satisfy_num"] = bond_cfg.satisfy_num
    return bond_condition

def config_to_star_condition(star_cfg: StarCondition) -> dict:
    star_condition = {}
    if star_cfg.checked:
        if (veins_point := get_veins_dict(star_cfg.veins_point_condition)):
            star_condition["veins_point"] = veins_point
        if (veins_amount := get_veins_dict(star_cfg.veins_amount_condition)):
            star_condition["veins_amount"] = veins_amount
        if len(star_cfg.star_type) > 0:
            star_condition["type"] = star_cfg.star_type
        if star_cfg.distance_level >= 0:
            star_condition["distance"] = star_cfg.distance_level
        if star_cfg.lumino_level > 0:
            star_condition["dyson_lumino"] = star_cfg.lumino_level
        if star_cfg.satisfy_num > 1:
            star_condition["satisfy_num"] = star_cfg.satisfy_num
    star_condition["planets"] = [config_to_planet_condition(planet_cfg) for planet_cfg in star_cfg.planet_condition]
    return star_condition

def config_to_planet_condition(planet_cfg: PlanetCondition) -> dict:
    planet_condition = {}
    if planet_cfg.checked:
        if (veins_point := get_veins_dict(planet_cfg.veins_point_condition)):
            planet_condition["veins_point"] = veins_point
        if (veins_amount := get_veins_dict(planet_cfg.veins_amount_condition)):
            planet_condition["veins_amount"] = veins_amount
        if len(planet_cfg.planet_type) > 0:
            planet_condition["type"] = planet_cfg.planet_type
        if len(planet_cfg.singularity) > 0:
            planet_condition["singularity"] = []
            for singularity in planet_cfg.singularity:
                if singularity == "潮汐锁定":
                    planet_condition["singularity"].append("潮汐锁定永昼永夜")
                else:
                    planet_condition["singularity"].append(singularity)
        if planet_cfg.liquid_type != "无限制":
            planet_condition["liquid"] = planet_cfg.liquid_type
        if planet_cfg.dsp_level != "无限制":
            planet_condition["dsp_level"] = planet_cfg.dsp_level
        if planet_cfg.satisfy_num > 1:
            planet_condition["satisfy_num"] = planet_cfg.satisfy_num
    planet_condition["moons"] = [config_to_planet_condition(moon_cfg) for moon_cfg in planet_cfg.moon_conditions]
    return planet_condition

def get_veins_dict(data: VeinsCondition) -> dict:
    veins = {}
    vein_names = VeinsName().model_dump()
    vein_data = data.model_dump()
    for key in vein_names.keys():
        if vein_data[key] > 0:
            veins[vein_names[key]] = vein_data[key]
    return veins
