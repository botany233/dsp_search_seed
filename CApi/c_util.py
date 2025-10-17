from . import search_seed

def check_planet_py(planet_data:dict, planet_condition:dict) -> bool:
    if "singularity" in planet_condition and planet_condition["singularity"] not in planet_data["singularity"].split("|"):
        return False
    if "type" in planet_condition and planet_condition["type"] != planet_data["type"]:
        return False
    if "liquid" in planet_condition and planet_condition["liquid"] != planet_data["liquid"]:
        return False
    if "veins" in planet_condition:
        if planet_data["type"] in ["气态巨星", "冰巨星"]:
            return False
        if not all(planet_data["veins"][i] >= planet_condition["veins"][i] for i in range(14)):
            return False
    return True

def check_star_py(star_data:dict, star_condition:dict) -> bool:
    if "type" in star_condition and star_condition["type"] != star_data["type"]:
        return False
    if "distance" in star_condition and star_condition["distance"] < star_data["distance"]:
        return False
    if "lumino" in star_condition and star_condition["lumino"] > star_data["dyson_lumino"]:
        return False
    if "veins" in star_condition:
        if not all(star_data["veins"][i] >= star_condition["veins"][i] for i in range(14)):
            return False
    if "planets" in star_condition:
        for planet_condition in star_condition["planets"]:
            if not any(check_planet_py(planet_data, planet_condition) for planet_data in star_data["planets"]):
                return False
    return True

def check_galaxy_py(galaxy_data:dict, galaxy_condition:dict) -> bool:
    if "veins" in galaxy_condition:
        if not all(galaxy_data["veins"][i] >= galaxy_condition["veins"][i] for i in range(14)):
            return False
    if "stars" in galaxy_condition:
        for star_condition in galaxy_condition["stars"]:
            if not any(check_star_py(star_data, star_condition) for star_data in galaxy_data["stars"]):
                return False
    return True

def check_seed_py(seed:int, star_num:int, galaxy_condition:dict) -> bool:
    galaxy_data = search_seed.get_galaxy_data(seed, star_num)
    return check_galaxy_py(galaxy_data, galaxy_condition)

def check_batch_py(start_seed:int, end_seed:int, star_num: int, galaxy_condition:dict)->list[str]:
    result = []
    for seed in range(start_seed, end_seed):
        if check_seed_py(seed, star_num, galaxy_condition):
            result.append(f"{star_num} {seed}")
    return result

vein_names = ["铁", "铜", "硅", "钛", "石", "煤", "油", "可燃冰", "金伯利",
              "分型硅", "有机晶体", "光栅石", "刺笋结晶", "单极磁石"]

def change_veins_to_legal(galaxy_condition:dict) -> dict:
    if "veins" in galaxy_condition:
        galaxy_veins = [0] * 14
        for key, value in galaxy_condition["veins"].items():
            galaxy_veins[vein_names.index(key)] = value
        galaxy_condition["veins"] = galaxy_veins

    for star_condition in galaxy_condition.get("stars", []):
        if "veins" in star_condition:
            star_veins = [0] * 14
            for key, value in star_condition["veins"].items():
                star_veins[vein_names.index(key)] = value
            star_condition["veins"] = star_veins

        for planet_condition in star_condition.get("planets", []):
            if "veins" in planet_condition:
                planet_veins = [0] * 14
                for key, value in planet_condition["veins"].items():
                    planet_veins[vein_names.index(key)] = value
                planet_condition["veins"] = planet_veins
    return galaxy_condition