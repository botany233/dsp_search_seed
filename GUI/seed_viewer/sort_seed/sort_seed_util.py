from CApi import *

from .custom_sort import handle_custom_sort
from .planet_sort import handle_planet_sort
from .star_sort import handle_star_sort
from .veins_point_sort import handle_veins_point_sort
from .veins_amount_sort import handle_veins_amount_sort

def get_value_function(maintype:str, subtype: str):
    if maintype == "矿脉储量":
        return handle_veins_amount_sort(subtype)
    elif maintype == "矿脉数量":
        return handle_veins_point_sort(subtype)
    elif maintype == "行星类别":
        return handle_planet_sort(subtype)
    elif maintype == "恒星类别":
        return handle_star_sort(subtype)
    elif maintype == "自定义":
        return handle_custom_sort(subtype)
