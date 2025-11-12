from CApi import *

from .custom_sort import handle_custom_sort
from .planet_sort import handle_planet_sort
from .star_sort import handle_star_sort
from .veins_group_sort import handle_veins_group_sort
from .veins_point_sort import handle_veins_point_sort

def get_value_function(maintype:str, subtype: str):
    if maintype == "矿簇":
        return handle_veins_group_sort(subtype)
    elif maintype == "矿脉":
        return handle_veins_point_sort(subtype)
    elif maintype == "行星类别":
        return handle_planet_sort(subtype)
    elif maintype == "恒星类别":
        return handle_star_sort(subtype)
    elif maintype == "自定义":
        return handle_custom_sort(subtype)

def get_seed_value(seed_id, star_num, value_func, quick):
    galaxy_data = get_galaxy_data_c(seed_id, star_num, quick)
    value = value_func(galaxy_data)
    return value
