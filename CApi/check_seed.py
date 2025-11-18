__all__ = ["check_seed_py"]

from .search_seed import *
from .const_values import *

def check_planet_py(planet_data:PlanetData, planet_condition:PlanetCondition) -> bool:
    if planet_condition.dsp_level > planet_data.dsp_level:
        return False
    if planet_condition.type and planet_condition.type != planet_data.type_id and\
        not (planet_condition.type == 23 and planet_data.type_id == 22):
        return False
    if (planet_condition.liquid & planet_data.liquid) != planet_condition.liquid:
        return False
    if (planet_condition.singularity & planet_data.singularity) != planet_condition.singularity:
        return False
    if planet_condition.need_veins:
        if any(planet_data.veins_group[i] < planet_condition.veins_group[i] for i in range(14)):
            return False
        if any(planet_data.veins_point[i] < planet_condition.veins_point[i] for i in range(14)):
            return False
    return True

def check_star_py(star_data:StarData, star_condition:StarCondition) -> bool:
    if star_condition.type and star_condition.type != star_data.type_id:
        return False
    if star_condition.distance < star_data.distance:
        return False
    if star_condition.dyson_lumino > star_data.dyson_lumino:
        return False
    if star_condition.need_veins:
        if any(star_data.veins_group[i] < star_condition.veins_group[i] for i in range(14)):
            return False
        if any(star_data.veins_point[i] < star_condition.veins_point[i] for i in range(14)):
            return False
    for planet_condition in star_condition.planets:
        left_satisfy_num = planet_condition.satisfy_num
        for planet_data in star_data.planets:
            if check_planet_py(planet_data, planet_condition):
                left_satisfy_num -= 1
                if not left_satisfy_num:
                    break
        if left_satisfy_num:
            return False
    return True

def check_galaxy_py(galaxy_data:GalaxyData, galaxy_condition:GalaxyCondition) -> bool:
    if galaxy_condition.need_veins:
        if any(galaxy_data.veins_group[i] < galaxy_condition.veins_group[i] for i in range(14)):
            return False
        if any(galaxy_data.veins_point[i] < galaxy_condition.veins_point[i] for i in range(14)):
            return False
    for star_condition in galaxy_condition.stars:
        left_satisfy_num = star_condition.satisfy_num
        for star_data in galaxy_data.stars:
            if check_star_py(star_data, star_condition):
                left_satisfy_num -= 1
                if not left_satisfy_num:
                    break
        if left_satisfy_num:
            return False
    for planet_condition in galaxy_condition.planets:
        left_satisfy_num = planet_condition.satisfy_num
        for star_data in galaxy_data.stars:
            if not left_satisfy_num:
                break
            for planet_data in star_data.planets:
                if check_planet_py(planet_data, planet_condition):
                    left_satisfy_num -= 1
                    if not left_satisfy_num:
                        break
        if left_satisfy_num:
            return False
    return True

def check_seed_py(seed:int, star_num:int, galaxy_condition:GalaxyCondition, quick: bool) -> bool:
    if not quick:
        if not check_galaxy_py(get_galaxy_data_c(seed, star_num, True), galaxy_condition):
            return False
    galaxy_data = get_galaxy_data_c(seed, star_num, quick)
    return check_galaxy_py(galaxy_data, galaxy_condition)
