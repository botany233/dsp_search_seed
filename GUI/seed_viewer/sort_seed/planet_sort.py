from CApi import GalaxyData

planet_types = ["地中海", "气态巨星", "冰巨星", "高产气巨", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩",
                "冰原冻土", "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩",
                "樱林海", "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]

def handle_planet_sort(subtype: str) -> function:
    n = planet_types.index(subtype)
    def get_planet_value(galaxy_data: GalaxyData) -> int:
        if n == 1:
            return galaxy_data.planet_type_nums[1] + galaxy_data.planet_type_nums[3]
        else:
            return galaxy_data.planet_type_nums[n]
    return get_planet_value