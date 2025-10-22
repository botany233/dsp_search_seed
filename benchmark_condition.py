def get_100k_factory_condition():
    planet_condition_1 = {"veins": {"金伯利":1, "刺笋结晶":1, "煤":1}, "type": "红石"} #增产剂
    planet_condition_2 = {"veins": {"分型硅":1, "油":1, "刺笋结晶":1, "煤":1}, "type": "潘多拉沼泽"} #紫糖
    planet_condition_3 = {"veins": {"光栅石":1}, "singularity": "卫星", "type": "橙晶荒漠"} #卡晶
    planet_condition_4 = {"is_in_dsp": True}

    star_condition_1 = {"planets": [planet_condition_1]}
    star_condition_2 = {"planets": [planet_condition_2]}
    star_condition_3 = {"planets": [planet_condition_3]}
    star_condition_4 = {"planets": [planet_condition_4], "type": "O型恒星", "satisfy_num": 2}

    galaxy_condition = {"stars":[star_condition_1, star_condition_2, star_condition_3, star_condition_4]}
    return galaxy_condition

def get_extreme_factory_condition():
    star_condition = {"type": "O型恒星", "satisfy_num": 5}
    
    galaxy_condition = {"stars":[star_condition], "planet_type_nums": {"水世界": 3}, "veins": {"单极磁石":24}}
    return galaxy_condition

def get_easy_condition():
    planet_condition_1 = {"singularity": "潮汐锁定永昼永夜"}
    planet_condition_2 = {"type": "气态巨星"}
    planet_condition_3 = {"liquid": "水"}
    planet_condition_4 = {"liquid": "硫酸"}
    planet_condition_5 = {"is_in_dsp": True}

    star_condition_1 = {"planets": [planet_condition_1, planet_condition_2, planet_condition_3, planet_condition_4, planet_condition_5],
                        "veins": {"油":1, "可燃冰":1, "金伯利":1, "分型硅":1, "有机晶体":1, "光栅石":1, "刺笋结晶":1},
                        "type": "O型恒星", "distance": 5}

    galaxy_condition = {"veins": {"单极磁石":4}, "stars":[star_condition_1]}
    return galaxy_condition

def get_3_blue_condition():
    star_condition = {"type": "蓝巨星", "satisfy_num": 3}
    galaxy_condition = {"stars":[star_condition]}
    return galaxy_condition

def get_impossible_condition():
    planet_condition = {"is_in_dsp": True, "satisfy_num": 2}
    star_condition = {"planets": [planet_condition]}
    galaxy_condition = {"stars":[star_condition]}
    return galaxy_condition