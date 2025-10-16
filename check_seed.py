from CApi import search_seed
import json

def check_planet(planet_data:dict, planet_condition:dict) -> bool:
    if "singularity" in planet_condition and planet_condition["singularity"] not in planet_data["singularity"].split("|"):
        return False
    if "type" in planet_condition and planet_condition["type"] != planet_data["type"]:
        return False
    if "liquid" in planet_condition and planet_condition["liquid"] != planet_data["liquid"]:
        return False
    if "veins" in planet_condition and planet_data["type"] not in ["气态巨星", "冰巨星"]:
        if not all(planet_data["veins"][key] >= planet_condition["veins"][key] for key in planet_condition["veins"].keys()):
            return False
    return True

def check_star(star_data:dict, star_condition:dict) -> bool:
    if "type" in star_condition and star_condition["type"] != star_data["type"]:
        return False
    if "distance" in star_condition and star_condition["distance"] < star_data["distance"]:
        return False
    if "lumino" in star_condition and star_condition["lumino"] > star_data["dyson_lumino"]:
        return False
    if "veins" in star_condition:
        if not all(star_data["veins"][key] >= star_condition["veins"][key] for key in star_condition["veins"].keys()):
            return False
    if "planets" in star_condition:
        for planet_condition in star_condition["planets"]:
            if not any(check_planet(planet_data, planet_condition) for planet_data in star_data["planets"]):
                return False
    return True

def check_galaxy(galaxy_data:dict, galaxy_condition:dict) -> bool:
    if "veins" in galaxy_condition:
        if not all(galaxy_data["veins"][key] >= galaxy_condition["veins"][key] for key in galaxy_condition["veins"].keys()):
            return False
    if "stars" in galaxy_condition:
        for star_condition in galaxy_condition["stars"]:
            if not any(check_star(star_data, star_condition) for star_data in galaxy_data["stars"]):
                return False
    return True

def check_seed(seed:int, star_num:int, galaxy_condition:dict) -> bool:
    galaxy_data = json.loads(search_seed.single_search(seed, star_num))
    return check_galaxy(galaxy_data, galaxy_condition)

if __name__ == "__main__":
    galaxy_data = json.loads(search_seed.single_search(233, 32))
    print(galaxy_data)