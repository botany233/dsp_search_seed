from .search_seed import *
from .const_values import *

__all__ = ["change_galaxy_condition_legal", "init_process"]

def init_process(device_id: int, local_size: int):
    do_init_c()
    if not set_device_id_c(device_id):
        raise Exception("Set device id failed!")
    set_local_size_c(local_size)

def change_galaxy_condition_legal(galaxy_condition:dict) -> dict:
    galaxy_condition = del_empty_condition(galaxy_condition)
    galaxy_condition["veins_group"] = change_veins_legal(galaxy_condition["veins_group"]) if "veins_group" in galaxy_condition else [0]*14
    galaxy_condition["veins_point"] = change_veins_legal(galaxy_condition["veins_point"]) if "veins_point" in galaxy_condition else [0]*14
    galaxy_condition["need_veins"] = get_need_veins(galaxy_condition)
    galaxy_condition["stars"] = [change_star_condition_legal(star_condition) for star_condition in galaxy_condition.get("stars", [])]
    galaxy_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in galaxy_condition.get("planets", [])]
    return galaxy_condition

def change_star_condition_legal(star_condition:dict) -> dict:
    star_condition["satisfy_num"] = star_condition["satisfy_num"] if "satisfy_num" in star_condition else 1
    star_condition["type"] = star_types_c.index(star_condition["type"]) + 1 if "type" in star_condition else 0
    star_condition["distance"] = star_condition["distance"] if "distance" in star_condition else 1000
    star_condition["dyson_lumino"] = star_condition["dyson_lumino"] if "dyson_lumino" in star_condition else 0
    star_condition["veins_group"] = change_veins_legal(star_condition["veins_group"]) if "veins_group" in star_condition else [0]*14
    star_condition["veins_point"] = change_veins_legal(star_condition["veins_point"]) if "veins_point" in star_condition else [0]*14
    star_condition["need_veins"] = get_need_veins(star_condition)
    star_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in star_condition.get("planets", [])]
    return star_condition

def change_planet_condition_legal(planet_condition:dict) -> dict:
    planet_condition["satisfy_num"] = planet_condition["satisfy_num"] if "satisfy_num" in planet_condition else 1
    planet_condition["dsp_level"] = dsp_level_c.index(planet_condition["dsp_level"]) + 1 if "dsp_level" in planet_condition else 0
    planet_condition["type"] = planet_types_c.index(planet_condition["type"]) + 1 if "type" in planet_condition else 0
    planet_condition["liquid"] = liquid_types_c.index(planet_condition["liquid"]) + 1 if "liquid" in planet_condition else 0
    planet_condition["singularity"] = change_singularity_legal(planet_condition["singularity"]) if "singularity" in planet_condition else 0
    planet_condition["veins_group"] = change_veins_legal(planet_condition["veins_group"]) if "veins_group" in planet_condition else [0]*14
    planet_condition["veins_point"] = change_veins_legal(planet_condition["veins_point"]) if "veins_point" in planet_condition else [0]*14
    planet_condition["need_veins"] = get_need_veins(planet_condition)
    return planet_condition

def change_veins_legal(veins:dict) -> list[int]:
    return [veins.get(vein_name, 0) for vein_name in vein_names_c]

def get_need_veins(condition: dict) -> int:
    need_veins = 0
    for i in range(14):
        if condition["veins_group"][i] > 0 or condition["veins_point"][i] > 0:
            need_veins |= (1 << i)
    return need_veins

def change_singularity_legal(singularity:str) -> int:
    return 1 << singularity_c.index(singularity)

def del_empty_condition(galaxy_condition:dict) -> dict:
    if "stars" in galaxy_condition:
        for i in range(len(galaxy_condition["stars"])-1, -1, -1):
            star_condition = galaxy_condition["stars"][i]
            if "planets" in star_condition:
                for j in range(len(star_condition["planets"])-1, -1, -1):
                    if not star_condition["planets"][j]:
                        star_condition["planets"].pop(j)
                if not star_condition["planets"]:
                    star_condition.pop("planets")
            if not star_condition:
                galaxy_condition["stars"].pop(i)
    if "planets" in galaxy_condition:
        for j in range(len(galaxy_condition["planets"])-1, -1, -1):
            if not galaxy_condition["planets"][j]:
                galaxy_condition["planets"].pop(j)
        if not galaxy_condition["planets"]:
            galaxy_condition.pop("planets")
    return galaxy_condition
