from .search_seed import *
import copy
from .const_values import *

__all__ = ["change_galaxy_condition_legal", "get_galaxy_condition_simple"]

def change_galaxy_condition_legal(galaxy_condition:dict) -> dict:
    if "veins" in galaxy_condition:
        galaxy_condition["veins"] = change_veins_legal(galaxy_condition["veins"])
    galaxy_condition["stars"] = [change_star_condition_legal(star_condition) for star_condition in galaxy_condition.get("stars", [])]
    galaxy_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in galaxy_condition.get("planets", [])]
    return del_empty_condition(galaxy_condition)

def change_star_condition_legal(star_condition:dict) -> dict:
    if "satisfy_num" not in star_condition:
        star_condition["satisfy_num"] = 1
    if "veins" in star_condition:
        star_condition["veins"] = change_veins_legal(star_condition["veins"])
    star_condition["planets"] = [change_planet_condition_legal(planet_condition) for planet_condition in star_condition.get("planets", [])]
    return star_condition

def change_planet_condition_legal(planet_condition:dict) -> dict:
    if "satisfy_num" not in planet_condition:
        planet_condition["satisfy_num"] = 1
    if "liquid" in planet_condition:
        planet_condition["liquid"] = liquid_types_c.index(planet_condition["liquid"])
    if "veins" in planet_condition:
        planet_condition["veins"] = change_veins_legal(planet_condition["veins"])
    return planet_condition

def change_veins_legal(veins:dict) -> list:
    return [veins.get(vein_name, 0) for vein_name in vein_names_c]

def get_galaxy_condition_simple(galaxy_condition:dict) -> dict:
    galaxy_condition_simple = copy.deepcopy(galaxy_condition)
    galaxy_condition_simple.pop("veins", 0)
    for star_condition in galaxy_condition_simple.get("stars", []):
        star_condition.pop("veins", 0)
        for planet_condition in star_condition.get("planets", []):
            planet_condition.pop("veins", 0)
    for planet_condition in galaxy_condition_simple.get("planets", []):
        planet_condition.pop("veins", 0)
    return del_empty_condition(galaxy_condition_simple)

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
