# 0x1: 快速模式; 0x2: 标准模式

def debug_condition_star():
    star_condition_1 = {"distance": 10, "type": "蓝巨星", "satisfy_num": 1}
    star_condition_2 = {"distance": 15, "type": "O型恒星", "satisfy_num": 2, "dyson_lumino": 2.4}
    galaxy_condition = {"stars":[star_condition_1, star_condition_2]}
    return galaxy_condition, (0, 1999999), (64, 64), "无限", 2

def debug_condition_planet():
    planet_condition_1 = {"type": "水世界", "satisfy_num": 8}
    moon_condition = {"veins_point": {"铜":1, "钛":1, "煤":1}, "liquid": "硫酸"}
    planet_condition_2 = {"type": "气态巨星", "satisfy_num": 1, "singularity": ["多卫星", "反向自转"], "moons": [moon_condition]}
    galaxy_condition = {"planets":[planet_condition_1, planet_condition_2]}
    return galaxy_condition, (0, 999999), (64, 64), "无限", 1

def debug_condition_veins():
    galaxy_condition = {"veins_point": {"单极磁石": 200, "刺笋结晶": 400, "油": 200}, "veins_amount": {"单极磁石": 30000000}}
    return galaxy_condition, (0, 9999), (64, 64), "1倍", 3

def debug_condition_good_start():
    planet_condition_1 = {"singularity": "潮汐锁定永昼永夜"}
    planet_condition_2 = {"liquid": "水"}
    planet_condition_3 = {"liquid": "硫酸"}
    planet_condition_4 = {"dsp_level": "全接收"}

    star_condition_1 = {"planets": [planet_condition_1, planet_condition_2, planet_condition_3, planet_condition_4], "veins_point": {"油":1, "可燃冰":1, "金伯利":1, "分型硅":1, "有机晶体":1, "光栅石":1, "刺笋结晶":1}, "type": "O型恒星", "distance": 10}

    galaxy_condition = {"stars":[star_condition_1]}
    return galaxy_condition, (0, 999999), (64, 64), "无限", 2

def debug_condition_better_birthstar():
    moon_condition_1 = {"type": "地中海"}
    moon_condition_2 = {"type": "贫瘠荒漠", "veins_point": {"刺笋结晶": 1}}
    planet_condition = {"type": "冰巨星", "moons": [moon_condition_1, moon_condition_2]}
    star_condition = {"planets": [planet_condition], "distance": 0}
    galaxy_condition = {"stars":[star_condition]}
    return galaxy_condition, (0, 1999999), (64, 64), "无限", 1

def debug_condition_100k_factory():
    planet_condition_1 = {"veins_point": {"金伯利":20, "刺笋结晶":20, "煤":30}} #增产剂
    planet_condition_2 = {"veins_point": {"分型硅":30, "油":20, "刺笋结晶":30, "煤":20}} #紫糖
    planet_condition_3 = {"veins_point": {"光栅石":100}, "singularity": "卫星"} #卡晶
    planet_condition_4 = {"dsp_level": "全接收", "satisfy_num": 2}

    star_condition_4 = {"planets": [planet_condition_4], "type": "O型恒星", "satisfy_num": 2}

    galaxy_condition = {"stars":[star_condition_4],
                        "planets":[planet_condition_1, planet_condition_2, planet_condition_3]}
    return galaxy_condition, (0, 199999), (64, 64), "无限", 3

def debug_condition_extreme_factory():
    planet_condition = {"type": "水世界", "satisfy_num": 8}
    star_condition = {"type": "O型恒星", "satisfy_num": 4}
    galaxy_condition = {"stars":[star_condition], "planets": [planet_condition], "veins_point": {"刺笋结晶": 400, "单极磁石": 200}}
    return galaxy_condition, (0, 999999), (64, 64), "无限", 3

def debug_condition_nuclear():
    moon_condition_1 = {"veins_point": {"铜":1, "钛":1, "煤":1}, "liquid": "硫酸"}
    moon_condition_2 = {"satisfy_num": 3}
    planet_condition = {"type": "高产气巨", "moons": [moon_condition_1, moon_condition_2], "satisfy_num": 2}
    galaxy_condition = {"planets":[planet_condition]}
    return galaxy_condition, (0, 999999), (64, 64), "无限", 2

def debug_condition_9m_factory():
    planet_condition = {"type": ["地中海", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩", "冰原冻土", "贫瘠荒漠", "戈壁",
                "火山灰", "红石", "草原", "水世界", "黑石盐滩", "樱林海", "飓风石林", "猩红冰湖", "热带草原",
                "橙晶荒漠", "极寒冻土", "潘多拉沼泽"], "satisfy_num": 230}
    galaxy_condition = {"planets": [planet_condition], "veins_point": {"单极磁石":250}}
    return galaxy_condition, (0, 999999), (64, 64), "无限", 3

debug_condition_functions = [
    debug_condition_star, debug_condition_planet, debug_condition_veins,
    debug_condition_good_start, debug_condition_better_birthstar, debug_condition_100k_factory,
    debug_condition_extreme_factory, debug_condition_nuclear, debug_condition_9m_factory]
