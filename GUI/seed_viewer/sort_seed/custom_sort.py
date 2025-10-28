from CApi import GalaxyData, StarData, PlanetData

def handle_custom_sort(subtype: str) -> function:
    if subtype == "例子：种子号":
        return get_seed_value
    elif subtype == "例子：恒星数量":
        return get_star_num_value
    elif subtype == "自定义1":
        return get_custom1_value
    elif subtype == "自定义2":
        return get_custom2_value
    elif subtype == "自定义3":
        return get_custom3_value

#例子：使用种子号作为种子价值
def get_seed_value(galaxy_data: GalaxyData) -> int:
    return galaxy_data.seed

#例子：使用恒星数量作为种子价值
def get_star_num_value(galaxy_data: GalaxyData) -> int:
    return galaxy_data.star_num

# 你可以完成下边的函数，通过自定义逻辑返回种子的价值
# 返回类型需要支持使用str函数转化为字符串以及sorted函数排序，建议为float或int类型
# GalaxyData具体包含哪些内容可以在CApi/search_seed.pyi中查看，也可以使用get_example.py获取对应的json格式
def get_custom1_value(galaxy_data: GalaxyData) -> float|int:
    return 0

def get_custom2_value(galaxy_data: GalaxyData) -> float|int:
    return 0

def get_custom3_value(galaxy_data: GalaxyData) -> float|int:
    return 0
