from multiprocessing import cpu_count
from concurrent.futures import ProcessPoolExecutor
from CApi import get_galaxy_data_c

from .custom_sort import handle_custom_sort
from .planet_sort import handle_planet_sort
from .star_sort import handle_star_sort
from .veins_sort import handle_veins_sort

def get_value_function(maintype:str, subtype: str) -> function:
    if maintype == "矿物":
        return handle_veins_sort(subtype)
    elif maintype == "行星类别":
        return handle_planet_sort(subtype)
    elif maintype == "恒星类别":
        return handle_star_sort(subtype)
    elif maintype == "自定义":
        return handle_custom_sort(subtype)

def get_seed_value(seed_id, star_num, value_func):
    galaxy_data = get_galaxy_data_c(seed_id, star_num)
    value = value_func(galaxy_data)
    return value

def get_seed_sort(seed_list:list[tuple[int, int]], maintype:str, subtype:str, is_descending: bool) -> list[int]:
    value_func = get_value_function(maintype, subtype)
    futures = []
    values = []
    with ProcessPoolExecutor(max_workers = cpu_count()) as executor:
        for seed in seed_list:
            futures.append(executor.submit(get_seed_value, seed[0], seed[1], value_func))

        index = 0
        for future in futures:
            values.append((index, future.result()))
            index += 1

    values.sort(key=lambda x: x[1], reverse=is_descending)
    return [value[0] for value in values]
