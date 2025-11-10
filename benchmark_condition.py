def get_100k_factory_condition():
    planet_condition_1 = {"veins_point": {"金伯利":20, "刺笋结晶":72, "煤":78}, "type": "红石"} #增产剂
    planet_condition_2 = {"veins_point": {"分型硅":9, "油":14, "刺笋结晶":52, "煤":14}, "type": "潘多拉沼泽"} #紫糖
    planet_condition_3 = {"veins_point": {"光栅石":55}, "singularity": "卫星", "type": "橙晶荒漠"} #卡晶
    planet_condition_4 = {"dsp_level": 1, "satisfy_num": 2}

    star_condition_4 = {"planets": [planet_condition_4], "type": "O型恒星", "satisfy_num": 2}

    galaxy_condition = {"stars":[star_condition_4],
                        "planets":[planet_condition_1, planet_condition_2, planet_condition_3]}
    return galaxy_condition

def get_extreme_factory_condition():
    planet_condition = {"type": "水世界", "satisfy_num": 3}

    star_condition = {"type": "O型恒星", "satisfy_num": 5}

    galaxy_condition = {"stars":[star_condition], "planets": [planet_condition], "veins": {"单极磁石":20}, "veins_point": {"单极磁石": 300, "刺笋结晶": 600}}
    return galaxy_condition

def get_ttenyx_condition():
    star_condition = {"distance": 100, "veins_point": {"钛":30, "硅":30}}

    galaxy_condition = {"veins_point": {"单极磁石":350, "油":350}, "stars":[star_condition]}
    return galaxy_condition

def get_easy_condition():
    planet_condition_1 = {"singularity": "潮汐锁定永昼永夜"}
    planet_condition_2 = {"type": "气态巨星"}
    planet_condition_3 = {"liquid": "水"}
    planet_condition_4 = {"liquid": "硫酸"}
    planet_condition_5 = {"dsp_level": 2}

    star_condition_1 = {"planets": [planet_condition_1, planet_condition_2, planet_condition_3, planet_condition_4, planet_condition_5],
                        "veins": {"油":1, "可燃冰":1, "金伯利":1, "分型硅":1, "有机晶体":1, "光栅石":1, "刺笋结晶":1},
                        "type": "O型恒星", "distance": 5}

    galaxy_condition = {"veins": {"单极磁石":4}, "veins_point": {"单极磁石": 80}, "stars":[star_condition_1]}
    return galaxy_condition

def get_3_blue_condition():
    star_condition = {"type": "蓝巨星", "satisfy_num": 3}
    galaxy_condition = {"stars":[star_condition]}
    return galaxy_condition
