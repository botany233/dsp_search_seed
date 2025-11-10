from .search_seed import *
import copy
from .const_values import *

__all__ = ["change_galaxy_condition_legal"]

def change_galaxy_condition_legal(galaxy_condition:dict) -> dict:
    if "veins" in galaxy_condition:
        galaxy_condition["veins"] = change_veins_legal(galaxy_condition["veins"])
    if "veins_point" in galaxy_condition:
        galaxy_condition["veins_point"] = change_veins_legal(galaxy_condition["veins_point"])
    galaxy_condition["stars"] = [change_star_condition_legal(star_condition) for star_condition in galaxy_condition.get("stars", [])]
    galaxy_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in galaxy_condition.get("planets", [])]
    return del_empty_condition(galaxy_condition)

def change_star_condition_legal(star_condition:dict) -> dict:
    if "satisfy_num" not in star_condition:
        star_condition["satisfy_num"] = 1
    if "veins" in star_condition:
        star_condition["veins"] = change_veins_legal(star_condition["veins"])
    if "veins_point" in star_condition:
        star_condition["veins_point"] = change_veins_legal(star_condition["veins_point"])
    star_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in star_condition.get("planets", [])]
    return star_condition

def change_planet_condition_legal(planet_condition:dict) -> dict:
    if "satisfy_num" not in planet_condition:
        planet_condition["satisfy_num"] = 1
    if "liquid" in planet_condition:
        planet_condition["liquid"] = liquid_types_c.index(planet_condition["liquid"])
    if "veins" in planet_condition:
        planet_condition["veins"] = change_veins_legal(planet_condition["veins"])
    if "veins_point" in planet_condition:
        planet_condition["veins_point"] = change_veins_legal(planet_condition["veins_point"])
    if "dsp_level" in planet_condition:
        planet_condition["dsp_level"] = dsp_level_c.index(planet_condition["dsp_level"]) + 1
    return planet_condition

def change_veins_legal(veins:dict) -> list[int]:
    return [veins.get(vein_name, 0) for vein_name in vein_names_c]

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
