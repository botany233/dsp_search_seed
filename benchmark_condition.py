def benchmark_condition_level_1():
    star_condition = {"type": "蓝巨星", "satisfy_num": 3}
    galaxy_condition = {"stars":[star_condition]}
    return galaxy_condition, (0, 4999999), (64, 64), True

def benchmark_condition_level_2():
    planet_condition_1 = {"type": "水世界", "satisfy_num": 6}
    moon_condition = {"satisfy_num": 3}
    planet_condition_2 = {"type": "气态巨星", "satisfy_num": 2, "singularity": "多卫星", "moons": [moon_condition]}
    galaxy_condition = {"planets":[planet_condition_1, planet_condition_2]}
    return galaxy_condition, (0, 1999999), (64, 64), True

def benchmark_condition_level_3_quick():
    galaxy_condition = {"veins_point": {"单极磁石":250, "油":250}}
    return galaxy_condition, (0, 999999), (64, 64), True

def benchmark_condition_level_3_standard():
    galaxy_condition = {"veins_point": {"单极磁石":250, "油":250}}
    return galaxy_condition, (0, 9999), (64, 64), False

def benchmark_condition_level_4():
    galaxy_condition = {"veins_point": {"铁":30000}}
    return galaxy_condition, (0, 199), (64, 64), False

def benchmark_condition_better_birthstar():
    moon_condition_1 = {"type": "地中海"}
    moon_condition_2 = {"type": "贫瘠荒漠", "veins_point": {"刺笋结晶": 1}}
    planet_condition = {"type": "冰巨星", "moons": [moon_condition_1, moon_condition_2]}
    star_condition = {"planets": [planet_condition], "distance": 0}
    galaxy_condition = {"stars":[star_condition]}
    return galaxy_condition, (0, 1999999), (64, 64), False

def benchmark_condition_easy():
    planet_condition_1 = {"singularity": "潮汐锁定永昼永夜"}
    planet_condition_2 = {"type": "气态巨星"}
    planet_condition_3 = {"liquid": "水"}
    planet_condition_4 = {"liquid": "硫酸"}
    planet_condition_5 = {"dsp_level": "全包"}
    star_condition_1 = {"planets": [planet_condition_1, planet_condition_2, planet_condition_3, planet_condition_4, planet_condition_5], "veins_group": {"油":1, "可燃冰":1, "金伯利":1, "分型硅":1, "有机晶体":1, "光栅石":1, "刺笋结晶":1}, "type": "O型恒星", "distance": 5}
    galaxy_condition = {"veins_point": {"单极磁石": 80, "刺笋结晶": 400}, "stars":[star_condition_1]}
    return galaxy_condition, (0, 4999999), (64, 64), False

benchmark_condition_functions = [benchmark_condition_level_1, benchmark_condition_level_2, benchmark_condition_level_3_quick, benchmark_condition_level_3_standard, benchmark_condition_level_4, benchmark_condition_better_birthstar, benchmark_condition_easy]

def debug_condition_star():
    star_condition_1 = {"distance": 10, "type": "蓝巨星", "satisfy_num": 1}
    star_condition_2 = {"distance": 15, "type": "O型恒星", "satisfy_num": 2, "dyson_lumino": 2.4}
    galaxy_condition = {"stars":[star_condition_1, star_condition_2]}
    return galaxy_condition, (0, 1999999), (64, 64), True

def debug_condition_planet():
    planet_condition_1 = {"type": "水世界", "satisfy_num": 6}
    moon_condition = {"veins_point": {"铜":1, "钛":1, "煤":1}, "liquid": "硫酸"}
    planet_condition_2 = {"type": "气态巨星", "satisfy_num": 4, "singularity": "多卫星", "moons": [moon_condition]}
    galaxy_condition = {"planets":[planet_condition_1, planet_condition_2]}
    return galaxy_condition, (0, 999999), (64, 64), True

def debug_condition_veins():
    galaxy_condition = {"veins_point": {"单极磁石": 200, "刺笋结晶": 400, "油": 200}}
    return galaxy_condition, (0, 9999), (64, 64), False

def debug_condition_good_birth_star():
    planet_condition_1 = {"singularity": "潮汐锁定永昼永夜"}
    planet_condition_2 = {"liquid": "水"}
    planet_condition_3 = {"liquid": "硫酸"}
    planet_condition_4 = {"dsp_level": "全接收"}

    star_condition_1 = {"planets": [planet_condition_1, planet_condition_2, planet_condition_3, planet_condition_4], "veins_group": {"油":1, "可燃冰":1, "金伯利":1, "分型硅":1, "有机晶体":1, "光栅石":1, "刺笋结晶":1}, "type": "O型恒星", "distance": 10}

    galaxy_condition = {"stars":[star_condition_1]}
    return galaxy_condition, (0, 999999), (64, 64), False

def debug_condition_100k_factory():
    planet_condition_1 = {"veins_point": {"金伯利":20, "刺笋结晶":20, "煤":30}} #增产剂
    planet_condition_2 = {"veins_point": {"分型硅":30, "油":20, "刺笋结晶":30, "煤":20}} #紫糖
    planet_condition_3 = {"veins_point": {"光栅石":100}, "singularity": "卫星"} #卡晶
    planet_condition_4 = {"dsp_level": "全接收", "satisfy_num": 2}

    star_condition_4 = {"planets": [planet_condition_4], "type": "O型恒星", "satisfy_num": 2}

    galaxy_condition = {"stars":[star_condition_4],
                        "planets":[planet_condition_1, planet_condition_2, planet_condition_3]}
    return galaxy_condition, (0, 199999), (64, 64), False

def debug_condition_extreme_factory():
    planet_condition = {"type": "水世界", "satisfy_num": 8}
    star_condition = {"type": "O型恒星", "satisfy_num": 4}
    galaxy_condition = {"stars":[star_condition], "planets": [planet_condition], "veins_group": {"单极磁石":8}, "veins_point": {"刺笋结晶": 400}}
    return galaxy_condition, (0, 999999), (64, 64), False

def debug_condition_nuclear():
    moon_condition_1 = {"veins_point": {"铜":1, "钛":1, "煤":1}, "liquid": "硫酸"}
    moon_condition_2 = {"satisfy_num": 3}
    planet_condition = {"type": "高产气巨", "moons": [moon_condition_1, moon_condition_2], "satisfy_num": 2}
    galaxy_condition = {"planets":[planet_condition]}
    return galaxy_condition, (0, 999999), (64, 64), True

debug_condition_functions = [
    debug_condition_star, debug_condition_planet, debug_condition_veins,
    debug_condition_good_birth_star, debug_condition_100k_factory, debug_condition_extreme_factory,
    debug_condition_nuclear]
