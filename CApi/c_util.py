from .search_seed import *
from .const_values import *

__all__ = ["change_galaxy_condition_legal", "init_process"]

def init_process(device_id: int, local_size: int):
    do_init_c()
    if not set_device_id_c(device_id):
        print("Set device id failed! Roll back to cpu!")
    set_local_size_c(local_size)

def change_galaxy_condition_legal(galaxy_condition:dict) -> dict:
    galaxy_condition = del_empty_condition(galaxy_condition)
    galaxy_condition["veins_point"] = change_veins_legal(galaxy_condition.get("veins_point", {}))
    galaxy_condition["veins_amount"] = change_veins_legal(galaxy_condition.get("veins_amount", {}))
    galaxy_condition["need_veins"] = get_need_veins(galaxy_condition)
    galaxy_condition["stars"] = [change_star_condition_legal(star_condition) for star_condition in galaxy_condition.get("stars", [])]
    galaxy_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in galaxy_condition.get("planets", [])]
    return galaxy_condition

def change_star_condition_legal(star_condition:dict) -> dict:
    star_condition["satisfy_num"] = star_condition["satisfy_num"] if "satisfy_num" in star_condition else 1
    star_condition["type"] = get_star_type_mask(star_condition["type"]) if "type" in star_condition else (1<<16)-1
    star_condition["distance"] = star_condition["distance"] if "distance" in star_condition else 1000
    star_condition["dyson_lumino"] = star_condition["dyson_lumino"] if "dyson_lumino" in star_condition else 0
    star_condition["veins_point"] = change_veins_legal(star_condition.get("veins_point", {}))
    star_condition["veins_amount"] = change_veins_legal(star_condition.get("veins_amount", {}))
    star_condition["need_veins"] = get_need_veins(star_condition)
    star_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in star_condition.get("planets", [])]
    return star_condition

def change_planet_condition_legal(planet_condition:dict) -> dict:
    planet_condition["satisfy_num"] = planet_condition["satisfy_num"] if "satisfy_num" in planet_condition else 1
    planet_condition["dsp_level"] = dsp_level_c.index(planet_condition["dsp_level"]) + 1 if "dsp_level" in planet_condition else 0
    planet_condition["type"] = get_planet_type_mask(planet_condition["type"]) if "type" in planet_condition else (1<<32)-1
    planet_condition["liquid"] = liquid_types_c.index(planet_condition["liquid"]) + 1 if "liquid" in planet_condition else 0
    planet_condition["singularity"] = get_singularity_mask(planet_condition["singularity"]) if "singularity" in planet_condition else 0
    planet_condition["veins_point"] = change_veins_legal(planet_condition.get("veins_point", {}))
    planet_condition["veins_amount"] = change_veins_legal(planet_condition.get("veins_amount", {}))
    planet_condition["need_veins"] = get_need_veins(planet_condition)
    planet_condition["moons"] = [change_planet_condition_legal(moon_condition) for moon_condition in planet_condition.get("moons", [])]
    return planet_condition

def change_veins_legal(veins:dict) -> list[int]:
    return [veins.get(vein_name, 0) for vein_name in vein_names_c]

def get_need_veins(condition: dict) -> int:
    need_veins = 0
    for i in range(14):
        if condition["veins_point"][i] > 0 or condition["veins_amount"][i] > 0:
            need_veins |= (1 << i)
    return need_veins

def get_star_type_mask(star_type:str|list[str]) -> int:
    if isinstance(star_type, str):
        star_type = [star_type]
    result = 0
    for st in star_type:
        result |= 1 << star_types_c.index(st)
    return result

def get_planet_type_mask(planet_type:str|list[str]) -> int:
    if isinstance(planet_type, str):
        planet_type = [planet_type]
    # if "气态巨星" in planet_type and "高产气巨" not in planet_type:
    #     planet_type.append("高产气巨")
    result = 0
    for pt in planet_type:
        result |= 1 << planet_types_c.index(pt)
    return result

def get_singularity_mask(singularity:str|list[str]) -> int:
    if isinstance(singularity, str):
        singularity = [singularity]
    result = 0
    for s in singularity:
        result |= 1 << singularity_c.index(s)
    return result

def del_empty_condition(galaxy_condition:dict) -> dict:
    del_empty_stars(galaxy_condition)
    del_empty_planets(galaxy_condition)
    return galaxy_condition

def del_empty_stars(condition:dict) -> None:
    if "stars" in condition:
        for i in range(len(condition["stars"])-1, -1, -1):
            del_empty_planets(condition["stars"][i])
            if not condition["stars"][i]:
                condition["stars"].pop(i)
        if not condition["stars"]:
            condition.pop("stars")

def del_empty_planets(condition:dict) -> None:
    if "planets" in condition:
        for j in range(len(condition["planets"])-1, -1, -1):
            del_empty_moons(condition["planets"][j])
            if not condition["planets"][j]:
                condition["planets"].pop(j)
        if not condition["planets"]:
            condition.pop("planets")

def del_empty_moons(condition:dict) -> None:
    if "moons" in condition:
        for j in range(len(condition["moons"])-1, -1, -1):
            if not condition["moons"][j]:
                condition["moons"].pop(j)
        if not condition["moons"]:
            condition.pop("moons")
