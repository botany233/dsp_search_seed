from .search_seed import *
import copy

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

def change_condition_to_legal(galaxy_condition:dict) -> dict:
    vein_names = ["铁", "铜", "硅", "钛", "石", "煤", "油", "可燃冰", "金伯利",
                  "分型硅", "有机晶体", "光栅石", "刺笋结晶", "单极磁石"]
    planet_types = ["地中海", "冰巨星", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩", "冰原冻土", "贫瘠荒漠",
                    "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩", "樱林海", "飓风石林",
                    "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽", "高产气巨", "气态巨星"]
    star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
                  "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星"]
    liquid_names = ["无", "水", "硫酸"]

    if "veins" in galaxy_condition:
        galaxy_veins = [0] * 14
        for key, value in galaxy_condition["veins"].items():
            galaxy_veins[vein_names.index(key)] = value
        galaxy_condition["veins"] = galaxy_veins

    if "planet_type_nums" in galaxy_condition:
        planet_type_nums = [0] * 23
        for key, value in galaxy_condition["planet_type_nums"].items():
            planet_type_nums[planet_types.index(key)] = value
        galaxy_condition["planet_type_nums"] = planet_type_nums

    if "star_type_nums" in galaxy_condition:
        star_type_nums = [0] * 14
        for key, value in galaxy_condition["star_type_nums"].items():
            star_type_nums[star_types.index(key)] = value
        galaxy_condition["star_type_nums"] = star_type_nums

    for star_condition in galaxy_condition.get("stars", []):
        if "satisfy_num" not in star_condition:
            star_condition["satisfy_num"] = 1

        if "veins" in star_condition:
            star_veins = [0] * 14
            for key, value in star_condition["veins"].items():
                star_veins[vein_names.index(key)] = value
            star_condition["veins"] = star_veins

        for planet_condition in star_condition.get("planets", []):
            if "satisfy_num" not in planet_condition:
                planet_condition["satisfy_num"] = 1

            if "veins" in planet_condition:
                planet_veins = [0] * 14
                for key, value in planet_condition["veins"].items():
                    planet_veins[vein_names.index(key)] = value
                planet_condition["veins"] = planet_veins

            if "liquid" in planet_condition:
                planet_condition["liquid"] = liquid_names.index(planet_condition["liquid"])

    for planet_condition in galaxy_condition.get("planets", []):
        if "satisfy_num" not in planet_condition:
            planet_condition["satisfy_num"] = 1

        if "veins" in planet_condition:
            planet_veins = [0] * 14
            for key, value in planet_condition["veins"].items():
                planet_veins[vein_names.index(key)] = value
            planet_condition["veins"] = planet_veins

        if "liquid" in planet_condition:
            planet_condition["liquid"] = liquid_names.index(planet_condition["liquid"])
    return del_empty_condition(galaxy_condition)

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