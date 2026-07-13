from config.cfg_dict_tying import GalaxyExportCondition, StarExportCondition, PlanetExportCondition, CSVExportCondition
from CApi import *
from language import tr, tr_domain


def _csv_label(key: str) -> str:
    return tr(f"viewer.export.csv.{key}")


def _axis_labels(key: str) -> list[str]:
    return [_csv_label(key).format(axis=axis) for axis in "xyz"]


def _tr_values(domain: str, values: list[str]) -> list[str]:
    return [tr_domain(domain, value) for value in values]


def _tr_traits(values: list[str]) -> str:
    return "|".join(tr_domain("singularity", value) for value in values)

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
    mask_para = [cfg.star_name, cfg.star_type, cfg.star_lumino, cfg.star_distance, (cfg.star_location, 3), True, cfg.planet_type, cfg.singularity, cfg.dsp_level, (cfg.dsp_degree, 2), cfg.liquid, cfg.wind_usage, cfg.light_usage, (cfg.gas_veins, len(gas_veins_c)), (cfg.veins_point, len(vein_names_c)), (cfg.veins_amount, len(vein_names_c))]
    mask = get_mask(mask_para)

    dsp_name = [""] + _tr_values("dsp_levels", dsp_level_c)
    liquid_name = [""] + _tr_values("liquids", liquid_types_c)

    full_data = [[
        _csv_label("star_name"),
        _csv_label("star_type"),
        _csv_label("star_lumino"),
        _csv_label("star_distance"),
        *_axis_labels("star_coord"),
        _csv_label("name"),
        _csv_label("type"),
        _csv_label("traits"),
        _csv_label("dsp_level"),
        _csv_label("dsp_degree_raw"),
        _csv_label("dsp_degree_enhance"),
        _csv_label("liquid"),
        _csv_label("wind_usage"),
        _csv_label("light_usage"),
        *_tr_values("gas_veins", gas_veins_c),
        *_tr_values("veins", vein_names_c),
        *_tr_values("veins", vein_names_c),
    ]]
    for star in galaxy.stars:
        for planet in star.planets:
            planet_data = [
                star.name,
                tr_domain("star_types", star.type),
                round(star.dyson_lumino, 3),
                round(star.distance, 2),
                *star.pos_m,
                planet.name,
                tr_domain("planet_types", planet.type),
                _tr_traits(planet.singularity_str),
                dsp_name[planet.dsp_level],
                f">{planet.raw_dsp_degree:.0f}°",
                f">{planet.enhance_dsp_degree:.0f}°",
                liquid_name[planet.liquid],
                f"{planet.wind:.0%}",
                f"{planet.lumino:.0%}",
                *map(lambda x: round(x, 3), planet.gas_veins),
                *planet.veins_point,
                *planet.veins_amount,
            ]
            full_data.append(planet_data)

    full_data = [",".join(map(str, (i for i, j in zip(line, mask) if j))) for line in full_data]
    text = "\n".join(full_data)
    return text + "\n"

def get_star_text(galaxy: GalaxyData, cfg: StarExportCondition) -> str:
    mask_para = [True, cfg.type, cfg.distance, (cfg.location, 3), (cfg.liquid, len(liquid_types_c)+1), cfg.ds_lumino, cfg.ds_radius, (cfg.gas_veins, len(gas_veins_c)), (cfg.veins_point, len(vein_names_c)), (cfg.veins_amount, len(vein_names_c))]
    mask = get_mask(mask_para)
    full_data = [[
        _csv_label("name"),
        _csv_label("type"),
        _csv_label("distance"),
        *_axis_labels("coord"),
        _csv_label("no_liquid"),
        *_tr_values("liquids", liquid_types_c),
        _csv_label("lumino"),
        _csv_label("dyson_radius"),
        *_tr_values("gas_veins", gas_veins_c),
        *_tr_values("veins", vein_names_c),
        *_tr_values("veins", vein_names_c),
    ]]
    for star in galaxy.stars:
        star_data = [star.name, tr_domain("star_types", star.type), round(star.distance, 2), *star.pos_m, *star.liquid, round(star.dyson_lumino, 3), star.dyson_radius, *map(lambda x: round(x, 3), star.gas_veins), *star.veins_point, *star.veins_amount]
        full_data.append(star_data)

    full_data = [",".join(map(str, (i for i, j in zip(line, mask) if j))) for line in full_data]
    text = "\n".join(full_data)
    return text + "\n"

def get_galaxy_text(galaxy: GalaxyData, cfg: GalaxyExportCondition) -> str:
    data = [
        [
            (_csv_label("seed_id"), galaxy.seed_id),
            (_csv_label("star_count"), galaxy.star_num),
            (_csv_label("resource_rate"), tr_domain("resource_rates", resource_rate_c[galaxy.resource_index])),
        ],
    ]
    if cfg.star_types:
        star_type_nums = {}
        for star in galaxy.stars:
            star_type_nums[star.type] = 1 + star_type_nums.get(star.type, 0)
        data.append([(tr_domain("star_types", key), value) for key, value in star_type_nums.items()])

    if cfg.planet_types:
        planet_type_nums = {key: 0 for key in planet_types_c}
        for star in galaxy.stars:
            for planet in star.planets:
                planet_type_nums[planet.type] = 1 + planet_type_nums.get(planet.type, 0)
        data.append([(tr_domain("planet_types", key), value) for key, value in planet_type_nums.items()])

    if cfg.liquid or cfg.gas_veins:
        group = []
        if cfg.liquid:
            group.extend((i, j) for i, j in zip([_csv_label("no_liquid"), *_tr_values("liquids", liquid_types_c)], galaxy.liquid))
        if cfg.gas_veins:
            group.extend((i, round(j, 3)) for i, j in zip(_tr_values("gas_veins", gas_veins_c), galaxy.gas_veins))
        data.append(group)
    
    if cfg.veins_point:
        data.append([(tr_domain("veins", key), value) for key, value in zip(vein_names_c, galaxy.veins_point)])
    if cfg.veins_amount:
        data.append([(tr_domain("veins", key), value) for key, value in zip(vein_names_c, galaxy.veins_amount)])

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
