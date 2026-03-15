import random
from typing import Literal

from CApi import *

resource_rates = [0.1, 0.3, 0.5, 0.8, 1, 1.5, 2, 3, 5, 8, 100]

veins_point_limits = {
        "galaxy": {
            "铁": 40000,
            "铜": 40000,
            "硅": 20000,
            "钛": 35000,
            "石": 25000,
            "煤": 8000,
            "油": 500,
            "可燃冰": 8000,
            "金伯利": 5000,
            "分型硅": 1700,
            "有机晶体": 1500,
            "光栅石": 3500,
            "刺笋结晶": 2800,
            "单极磁石": 400
        },
        "star": {
            "铁": 800,
            "铜": 800,
            "硅": 400,
            "钛": 700,
            "石": 500,
            "煤": 160,
            "油": 10,
            "可燃冰": 160,
            "金伯利": 100,
            "分型硅": 34,
            "有机晶体": 30,
            "光栅石": 70,
            "刺笋结晶": 56,
            "单极磁石": 200
        },
        "planet": {
            "铁": 300,
            "铜": 300,
            "硅": 200,
            "钛": 200,
            "石": 150,
            "煤": 150,
            "油": 30,
            "可燃冰": 150,
            "金伯利": 150,
            "分型硅": 100,
            "有机晶体": 150,
            "光栅石": 200,
            "刺笋结晶": 200,
            "单极磁石": 200
        }
    }

veins_amount_limits = {
        "galaxy": {
            "铁": 10 * (10 ** 9),
            "铜": 12 * (10 ** 9),
            "硅": 6 * (10 ** 9),
            "钛": 10 * (10 ** 9),
            "石": 8 * (10 ** 9),
            "煤": 2 * (10 ** 9),
            "油": 150 * (10 ** 6),
            "可燃冰": 2.5 * (10 ** 9),
            "金伯利": 1.4 * (10 ** 9),
            "分型硅": 450 * (10 ** 6),
            "有机晶体": 400 * (10 ** 6),
            "光栅石": 1 * (10 ** 9),
            "刺笋结晶": 600 * (10 ** 6),
            "单极磁石": 80 * (10 ** 6)
        },
        "star": {
            "铁": 10 * (10 ** 7),
            "铜": 12 * (10 ** 7),
            "硅": 6 * (10 ** 7),
            "钛": 10 * (10 ** 7),
            "石": 8 * (10 ** 7),
            "煤": 2 * (10 ** 7),
            "油": 150 * (10 ** 4),
            "可燃冰": 2.5 * (10 ** 7),
            "金伯利": 1.4 * (10 ** 7),
            "分型硅": 450 * (10 ** 4),
            "有机晶体": 400 * (10 ** 4),
            "光栅石": 1 * (10 ** 7),
            "刺笋结晶": 600 * (10 ** 4),
            "单极磁石": 80 * (10 ** 4)
        },
        "planet": {
            "铁": 10 * (10 ** 6),
            "铜": 12 * (10 ** 6),
            "硅": 6 * (10 ** 6),
            "钛": 10 * (10 ** 6),
            "石": 8 * (10 ** 6),
            "煤": 2 * (10 ** 6),
            "油": 150 * (10 ** 3),
            "可燃冰": 2.5 * (10 ** 6),
            "金伯利": 1.4 * (10 ** 6),
            "分型硅": 450 * (10 ** 3),
            "有机晶体": 400 * (10 ** 3),
            "光栅石": 1 * (10 ** 6),
            "刺笋结晶": 600 * (10 ** 3),
            "单极磁石": 40 * (10 ** 6)
        }
    }

def get_planet_condition(resource_index: int, need_veins: bool, quick: bool):
    planet_condition = {}
    planet_condition["satisfy_num"] = random.randint(1, 3)
    if not need_veins:
        if random.random() < 0.1 and planet_condition["satisfy_num"] <= 2:
            planet_condition["dsp_level"] = dsp_level_c[0]
        elif random.random() < 0.1 and planet_condition["satisfy_num"] <= 1:
            planet_condition["dsp_level"] = dsp_level_c[1]
        if random.random() < 1:
            planet_condition["type"] = random.sample(planet_types_c, k=random.randint(3, 5))
        if random.random() < 0.2:
            planet_condition["liquid"] = random.choice(liquid_types_c)
            if planet_condition["liquid"] == "硫酸" and "type" in planet_condition:
                planet_condition["type"].append("火山灰")
        if random.random() < 0.2:
            planet_condition["singularity"] = random.sample(singularity_c, k=random.randint(1, 2))
    if random.random() < 0.1:
        planet_condition["moons"] = [get_planet_condition(resource_index, need_veins, quick) for _ in range(random.randint(1, 2))]
    if need_veins and "moons" not in planet_condition:
        veins_point_condition, veins_amount_condition = get_veins_condition("planet", resource_index, quick)
        planet_condition["veins_point"] = veins_point_condition
        if resource_rate_c[resource_index] != "无限":
            planet_condition["veins_amount"] = veins_amount_condition
    return planet_condition

def get_star_condition(resource_index: int, need_veins: bool, quick: bool):
    star_condition = {}
    star_condition["satisfy_num"] = random.randint(1, 3)
    if not need_veins:
        if random.random() < 1:
            star_condition["type"] = random.sample(star_types_c, k=random.randint(3, 5))
        if random.random() < 0.3:
            star_condition["distance"] = random.uniform(0, 100)
        if random.random() < 0.3:
            star_condition["dyson_lumino"] = random.uniform(0, 2.4)
            if "type" in star_condition:
                star_condition["type"].append("O型恒星")
        if random.random() < 0.5:
            star_condition["planets"] = [get_planet_condition(resource_index, need_veins, quick) for _ in range(random.randint(1, 2))]
    if need_veins:
        veins_point_condition, veins_amount_condition = get_veins_condition("star", resource_index, quick)
        star_condition["veins_point"] = veins_point_condition
        if resource_rate_c[resource_index] != "无限":
            star_condition["veins_amount"] = veins_amount_condition
    return star_condition

def get_galaxy_condition(resource_index: int, need_veins: bool, quick: bool):
    galaxy_condition = {}
    if random.random() < 0.95 and not need_veins:
        galaxy_condition["stars"] = [get_star_condition(resource_index, need_veins, quick) for _ in range(random.randint(1, 2))]
    if random.random() < 0.6 or "stars" not in galaxy_condition:
        galaxy_condition["planets"] = [get_planet_condition(resource_index, need_veins, quick) for _ in range(random.randint(1, 3))]
    if need_veins:
        veins_point_condition, veins_amount_condition = get_veins_condition("galaxy", resource_index, quick)
        galaxy_condition["veins_point"] = veins_point_condition
        if resource_rate_c[resource_index] != "无限":
            galaxy_condition["veins_amount"] = veins_amount_condition
    return galaxy_condition

def get_veins_condition(satro_type: Literal["galaxy", "star", "planet"], resource_index: int, quick: bool):
    point_rate = 0.7 if quick else 0.6
    amount_rate = 0.7 if quick else 0.5
    resource_rate = resource_rates[resource_index]
    vein_names = random.sample(vein_names_c, k=random.randint(1, 2))

    veins_point_limit = veins_point_limits[satro_type]
    veins_point_limit = {key: veins_point_limit[key] for key in vein_names}
    veins_point_condition = {vein: random.randint(1, int(limit*point_rate)) for vein, limit in veins_point_limit.items()}

    veins_amount_limit = veins_amount_limits[satro_type]
    veins_amount_limit = {key: veins_amount_limit[key] for key in vein_names}
    veins_amount_condition = {vein: random.randint(1, int(limit*amount_rate*resource_rate)) for vein, limit in veins_amount_limit.items()}
    return veins_point_condition, veins_amount_condition

def get_random_debug_condition():
    if random.random() < 0.3:
        need_veins = True
    else:
        need_veins = False
    if need_veins:
        quick = random.choice([True, False])
    else:
        quick = True
    resource_index = random.randint(0, len(resource_rate_c)-1)
    if need_veins:
        if quick:
            seeds = (0, 99999)
        else:
            seeds = (0, 499)
    else:
        seeds = (0, 199999) 
    galaxy_condition = get_galaxy_condition(resource_index, need_veins, quick)
    return galaxy_condition, seeds, (64, 64), resource_index, quick

__all__ = ["get_random_debug_condition"]
