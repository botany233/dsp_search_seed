from config.cfg_dict_tying import GalaxyExportCondition, StarExportCondition, PlanetExportCondition, CSVExportCondition
from CApi import *

def get_mask(mask_pras: list) -> list[bool]:
    mask = []
    for i in mask_pras:
        if isinstance(i, bool):
            mask.append(i)
        elif isinstance(i, (tuple, list)):
            mask.extend(i[0] for _ in range(i[1]))
        else:
            raise Exception()
    return mask

def get_planet_text(galaxy: GalaxyData, cfg: PlanetExportCondition) -> str:
    mask_para = [cfg.star_name, cfg.star_type, cfg.star_lumino, cfg.star_distance, (cfg.star_location, 3), True, cfg.planet_type, cfg.singularity, cfg.dsp_level, cfg.liquid, cfg.wind_usage, cfg.light_usage, (cfg.gas_veins, len(gas_veins_c)), (cfg.veins, len(vein_names_c))]
    mask = get_mask(mask_para)

    dsp_name = [""] + dsp_level_c
    liquid_name = [""] + liquid_types_c

    full_data = [["恒星名称", "恒星类型", "恒星亮度", "恒星距离", *[f"恒星坐标_{i}" for i in "xyz"], "名称", "类型", "词条", "戴森球接收", "液体", "风能利用率", "光能利用率", *gas_veins_c, *vein_names_c]]
    for star in galaxy.stars:
        for planet in star.planets:
            planet_data = [star.name, star.type, round(star.dyson_lumino,3), round(star.distance,2), *star.pos, planet.name, planet.type, "|".join(planet.singularity_str), dsp_name[planet.dsp_level], liquid_name[planet.liquid], f"{planet.wind:.0%}", f"{planet.lumino:.0%}", *map(lambda x: round(x,3),planet.gas_veins), *planet.veins_point]
            full_data.append(planet_data)

    full_data = [",".join(map(str, (i for i, j in zip(line, mask) if j))) for line in full_data]
    text = "\n".join(full_data)
    return text + "\n"

def get_star_text(galaxy: GalaxyData, cfg: StarExportCondition) -> str:
    mask_para = [True, cfg.type, cfg.distance, (cfg.location, 3), (cfg.liquid, len(liquid_types_c)+1), cfg.ds_lumino, cfg.ds_radius, (cfg.gas_veins, len(gas_veins_c)), (cfg.veins, len(vein_names_c))]
    mask = get_mask(mask_para)
    full_data = [["名称", "类型", "距离", *[f"坐标_{i}" for i in "xyz"], "无液体", *liquid_types_c, "亮度", "戴森球半径", *gas_veins_c, *vein_names_c]]
    for star in galaxy.stars:
        star_data = [star.name, star.type, round(star.distance,2), *star.pos, *star.liquid, round(star.dyson_lumino,3), star.dyson_radius, *map(lambda x: round(x,3),star.gas_veins), *star.veins_point]
        full_data.append(star_data)

    full_data = [",".join(map(str, (i for i, j in zip(line, mask) if j))) for line in full_data]
    text = "\n".join(full_data)
    return text + "\n"

def get_galaxy_text(galaxy: GalaxyData, cfg: GalaxyExportCondition) -> str:
    data = [
        [
            ("种子id", galaxy.seed),
            ("恒星数", galaxy.star_num),
        ],
    ]
    if cfg.star_types:
        star_type_nums = {}
        for star in galaxy.stars:
            star_type_nums[star.type] = 1 + star_type_nums.get(star.type, 0)
        data.append([(key, value) for key, value in star_type_nums.items()])

    if cfg.planet_types:
        planet_type_nums = {key: 0 for key in ["地中海", "气态巨星", "高产气巨", "冰巨星", "熔岩", "干旱荒漠", "灰烬冻土", "海洋丛林", "冰原冻土", "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩", "樱林海", "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]}
        for star in galaxy.stars:
            for planet in star.planets:
                planet_type_nums[planet.type] += 1
        data.append([(key, value) for key, value in planet_type_nums.items()])

    if cfg.liquid or cfg.gas_veins:
        group = []
        if cfg.liquid:
            group.extend((i, j) for i, j in zip(["无液体"] + liquid_types_c, galaxy.liquid))
        if cfg.gas_veins:
            group.extend((i, round(j,3)) for i, j in zip(gas_veins_c, galaxy.gas_veins))
        data.append(group)
    
    if cfg.veins:
        data.append([(key, value) for key, value in zip(vein_names_c, galaxy.veins_point)])

    text = ""
    for piece in data:
        title, value = zip(*piece)
        text += ",".join(map(str, title)) + "\n"
        text += ",".join(map(str, value)) + "\n"
    return text

def get_seed_text(galaxy: GalaxyData, cfg: CSVExportCondition) -> str:
    texts = []
    if cfg.galaxy.enable:
        texts.append(get_galaxy_text(galaxy, cfg.galaxy))
    if cfg.star.enable:
        texts.append(get_star_text(galaxy, cfg.star))
    if cfg.planet.enable:
        texts.append(get_planet_text(galaxy, cfg.planet))
    return "\n".join(texts)

def save_seed_info(save_path: str, galaxy: GalaxyData, cfg: CSVExportCondition) -> None:
    save_text = get_seed_text(galaxy, cfg)
    with open(save_path, "w", encoding="utf-8-sig") as f:
        f.write(save_text)
