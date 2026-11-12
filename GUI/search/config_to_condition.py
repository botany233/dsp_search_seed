__all__ = ["config_to_galaxy_condition", "config_to_advanced_condition"]

from config.cfg_dict_tying import (
    GalaxyCondition,
    PlanetCondition,
    StarCondition,
    VeinsCondition,
    VeinsName,
    AdvancedNode, AdvancedEndpoint,
)
from language import tr


def config_to_endpoint(endpoint: AdvancedEndpoint) -> dict:
    if endpoint.kind == "star":
        return {"type": "恒星", "value": config_to_star_condition(endpoint.star)}
    return {"type": "星球", "value": config_to_planet_condition(endpoint.planet)}


def config_to_advanced_condition(node: AdvancedNode) -> dict:
    if node.kind in ("logic", "bond", "mineral") and not node.checked:
        return {"type": "逻辑", "value": {"satisfy_num": 0, "children": []}}
    if node.kind == "mineral":
        return {"type": "矿物", "value": {
            "veins_point": get_veins_dict(node.veins_point_condition),
            "veins_amount": get_veins_dict(node.veins_amount_condition),
        }}
    if node.kind == "star":
        return {"type": "恒星", "value": config_to_star_condition(node.condition)}
    if node.kind == "planet":
        return {"type": "星球", "value": config_to_planet_condition(node.condition)}
    if node.kind == "bond":
        return {"type": "绑定", "value": {
            "distance": node.distance, "satisfy_num": node.satisfy_num,
            "con1": config_to_endpoint(node.con1), "con2": config_to_endpoint(node.con2),
        }}
    count = len(node.children)
    if count == 0:
        raise ValueError(tr("search.advanced.empty_group"))
    if node.operation == "not" and count != 1:
        raise ValueError(tr("search.advanced.not_one"))
    if node.operation == "at_least" and not 1 <= node.minimum <= count:
        raise ValueError(tr("search.advanced.invalid_minimum").format(count=count))
    thresholds = {"all": count, "any": 1, "at_least": node.minimum, "not": 0}
    return {"type": "逻辑", "value": {
        "satisfy_num": thresholds[node.operation],
        "children": [config_to_advanced_condition(child) for child in node.children],
    }}

def config_to_galaxy_condition(galaxy_cfg: GalaxyCondition) -> dict:
    mineral_condition = {}
    if galaxy_cfg.checked:
        if (veins_point := get_veins_dict(galaxy_cfg.veins_point_condition)):
            mineral_condition["veins_point"] = veins_point
        if (veins_amount := get_veins_dict(galaxy_cfg.veins_amount_condition)):
            mineral_condition["veins_amount"] = veins_amount
    children = []
    if mineral_condition:
        children.append({"type": "矿物", "value": mineral_condition})
    children.extend({"type": "恒星", "value": config_to_star_condition(star_cfg)} for star_cfg in galaxy_cfg.star_condition)
    children.extend({"type": "星球", "value": config_to_planet_condition(planet_cfg)} for planet_cfg in galaxy_cfg.planet_condition)
    galaxy_condition = {
        "type": "逻辑",
        "value": {"satisfy_num": len(children), "children": children},
    }
    if not galaxy_cfg.valid_state:
        galaxy_condition = {"type": "逻辑", "value": {"satisfy_num": 0, "children": [galaxy_condition]}}
    return galaxy_condition

# def config_to_bond_condition(bond_cfg: BondCondition) -> dict:
#     bond_condition = {
#         "con1": {
#             "type": "星球" if bond_cfg.con1_is_planet else "恒星",
#             "value": config_to_planet_condition(bond_cfg.con1_planet) if bond_cfg.con1_is_planet else config_to_star_condition(bond_cfg.con1_star),
#         },
#         "con2": {
#             "type": "星球" if bond_cfg.con2_is_planet else "恒星",
#             "value": config_to_planet_condition(bond_cfg.con2_planet) if bond_cfg.con2_is_planet else config_to_star_condition(bond_cfg.con2_star),
#         },
#     }
#     if bond_cfg.checked:
#         if bond_cfg.distance >= 0:
#             bond_condition["distance"] = bond_cfg.distance
#         if bond_cfg.satisfy_num > 1:
#             bond_condition["satisfy_num"] = bond_cfg.satisfy_num
#     else:
#         bond_condition["satisfy_num"] = 0
#     return bond_condition

def config_to_star_condition(star_cfg: StarCondition) -> dict:
    star_condition = {
        "planets": [config_to_planet_condition(planet_cfg) for planet_cfg in star_cfg.planet_condition],
    }
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
    return star_condition

def config_to_planet_condition(planet_cfg: PlanetCondition) -> dict:
    planet_condition = {
        "moons": [config_to_planet_condition(moon_cfg) for moon_cfg in planet_cfg.moon_conditions]
    }
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
    return planet_condition

def get_veins_dict(data: VeinsCondition) -> dict:
    veins = {}
    vein_names = VeinsName().model_dump()
    vein_data = data.model_dump()
    for key in vein_names.keys():
        if vein_data[key] > 0:
            veins[vein_names[key]] = vein_data[key]
    return veins
