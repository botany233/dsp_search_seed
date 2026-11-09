def benchmark_condition_level_1():
    condition = {"type": 2, "value": {"type": ["B型蓝巨星", "O型蓝巨星"], "satisfy_num": 3}}
    return condition, (0, 4999999), (64, 64), True


def benchmark_condition_level_2():
    planet_1 = {"type": 3, "value": {"type": "水世界", "satisfy_num": 6}}
    planet_2 = {"type": 3, "value": {
        "type": "气态巨星", "satisfy_num": 2, "singularity": "多卫星",
        "moons": [{"satisfy_num": 3}],
    }}
    condition = {"type": 5, "value": {"children": [planet_1, planet_2], "satisfy_num": 2}}
    return condition, (0, 1999999), (64, 64), True


def benchmark_condition_level_3_quick():
    condition = {"type": 1, "value": {"veins_point": {"单极磁石": 250, "油": 250}}}
    return condition, (0, 999999), (64, 64), True


def benchmark_condition_level_3_standard():
    condition = {"type": 1, "value": {"veins_point": {"单极磁石": 250, "油": 250}}}
    return condition, (0, 299999), (64, 64), False


def benchmark_condition_level_4():
    condition = {"type": 1, "value": {"veins_point": {"铁": 30000}}}
    return condition, (0, 1999), (64, 64), False


def benchmark_condition_better_birthstar():
    moon_1 = {"type": "地中海"}
    moon_2 = {"type": "贫瘠荒漠", "veins_point": {"刺笋结晶": 1, "可燃冰": 1}}
    planet = {"type": ["气态巨星", "高产气巨"], "moons": [moon_1, moon_2]}
    condition = {"type": 2, "value": {"planets": [planet], "distance": 0}}
    return condition, (0, 4999999), (64, 64), False


def benchmark_condition_easy():
    star = {"type": 2, "value": {
        "planets": [
            {"singularity": "潮汐锁定永昼永夜"}, {"type": "气态巨星"},
            {"liquid": "水"}, {"liquid": "硫酸"}, {"dsp_level": "全包"},
        ],
        "veins_point": {"油": 1, "可燃冰": 1, "金伯利": 1, "分形硅": 1,
                        "有机晶体": 1, "光栅石": 1, "刺笋结晶": 1},
        "type": "O型恒星", "distance": 5,
    }}
    mineral = {"type": 1, "value": {"veins_point": {"单极磁石": 80, "刺笋结晶": 400}}}
    condition = {"type": 5, "value": {"children": [mineral, star], "satisfy_num": 2}}
    return condition, (0, 4999999), (64, 64), False


def benchmark_condition_better_develop():
    star = {"type": 2, "value": {
        "planets": [{"type": "高产气巨"}, {"dsp_level": "全接收"}, {"liquid": "硫酸"}, {"liquid": "水"}],
        "veins_point": {"油": 1, "可燃冰": 1, "金伯利": 1, "分形硅": 1,
                        "有机晶体": 1, "光栅石": 1, "刺笋结晶": 1},
        "type": "O型恒星",
    }}
    planet = {"type": 3, "value": {"veins_point": {"单极磁石": 1}}}
    bond = {"type": 4, "value": {"con1": star, "con2": planet, "satisfy_num": 1, "distance": 12}}
    mineral = {"type": 1, "value": {"veins_point": {"单极磁石": 120}}}
    condition = {"type": 5, "value": {"children": [bond, mineral], "satisfy_num": 2}}
    return condition, (0, 1999999), (64, 64), False


benchmark_condition_functions = [
    benchmark_condition_level_1, benchmark_condition_level_2,
    benchmark_condition_level_3_quick, benchmark_condition_level_3_standard,
    benchmark_condition_level_4, benchmark_condition_better_birthstar,
    benchmark_condition_easy, benchmark_condition_better_develop,
]
