from .search_seed import *
import copy

__all__ = ["check_batch_py", "change_galaxy_condition_legal", "get_galaxy_condition_simple"]

def check_planet_py(planet_data:PlanetData, planet_condition:dict) -> bool:
    if "is_in_dsp" in planet_condition and not planet_data.is_in_dsp:
        return False
    if "is_on_dsp" in planet_condition and not planet_data.is_on_dsp:
        return False
    if "singularity" in planet_condition and planet_condition["singularity"] not in planet_data.singularity:
        return False
    if "type" in planet_condition and planet_condition["type"] != planet_data.type and\
        not (planet_condition["type"] == "气态巨星" and planet_data.type == "高产气巨"):
        return False
    if "liquid" in planet_condition and planet_condition["liquid"] != planet_data.liquid:
        return False
    if "veins" in planet_condition:
        if planet_data.is_gas:
            return False
        if not all(planet_data.veins[i] >= planet_condition["veins"][i] for i in range(14)):
            return False
    return True

def check_star_py(star_data:StarData, star_condition:dict) -> bool:
    if "type" in star_condition and star_condition["type"] != star_data.type:
        return False
    if "distance" in star_condition and star_condition["distance"] < star_data.distance:
        return False
    if "lumino" in star_condition and star_condition["lumino"] > star_data.dyson_lumino:
        return False
    if "veins" in star_condition:
        if not all(star_data.veins[i] >= star_condition["veins"][i] for i in range(14)):
            return False
    if "planets" in star_condition:
        for planet_condition in star_condition["planets"]:
            left_satisfy_num = planet_condition["satisfy_num"]
            for planet_data in star_data.planets:
                if check_planet_py(planet_data, planet_condition):
                    left_satisfy_num -= 1
                    if not left_satisfy_num:
                        break
            if left_satisfy_num:
                return False
    return True

def check_galaxy_py(galaxy_data:GalaxyData, galaxy_condition:dict) -> bool:
    if "veins" in galaxy_condition:
        if not all(galaxy_data.veins[i] >= galaxy_condition["veins"][i] for i in range(14)):
            return False
    if "stars" in galaxy_condition:
        for star_condition in galaxy_condition["stars"]:
            left_satisfy_num = star_condition["satisfy_num"]
            for star_data in galaxy_data.stars:
                if check_star_py(star_data, star_condition):
                    left_satisfy_num -= 1
                    if not left_satisfy_num:
                        break
            if left_satisfy_num:
                return False
    if "planets" in galaxy_condition:
        for planet_condition in galaxy_condition["planets"]:
            left_satisfy_num = planet_condition["satisfy_num"]
            for star_data in galaxy_data.stars:
                if not left_satisfy_num:
                    break
                for planet_data in star_data.planets:
                    if check_planet_py(planet_data, planet_condition):
                        left_satisfy_num -= 1
                        if not left_satisfy_num:
                            break
            if left_satisfy_num:
                return False
    return True

def check_seed_py(seed:int, star_num:int, galaxy_condition:dict) -> bool:
    galaxy_data = get_galaxy_data_c(seed, star_num)
    return check_galaxy_py(galaxy_data, galaxy_condition)

def check_batch_py(start_seed:int, end_seed:int, start_star_num: int, end_star_num: int, galaxy_condition:dict)->list[str]:
    result = []
    for seed in range(start_seed, end_seed):
        for star_num in range(start_star_num, end_star_num):
            if check_seed_py(seed, star_num, galaxy_condition):
                result.append(f"{seed}, {star_num}")
    return result

vein_names = ["铁", "铜", "硅", "钛", "石", "煤", "油", "可燃冰", "金伯利",
              "分型硅", "有机晶体", "光栅石", "刺笋结晶", "单极磁石"]
liquid_names = ["无", "水", "硫酸"]

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
        planet_condition["liquid"] = liquid_names.index(planet_condition["liquid"])
    if "veins" in planet_condition:
        planet_condition["veins"] = change_veins_legal(planet_condition["veins"])
    return planet_condition

def change_veins_legal(veins:dict) -> list:
    return [veins.get(vein_name, 0) for vein_name in vein_names]

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