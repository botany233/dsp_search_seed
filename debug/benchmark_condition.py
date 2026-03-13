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

benchmark_condition_functions = [
    benchmark_condition_level_1, benchmark_condition_level_2, benchmark_condition_level_3_quick, benchmark_condition_level_3_standard, benchmark_condition_level_4,
    benchmark_condition_better_birthstar, benchmark_condition_easy]
