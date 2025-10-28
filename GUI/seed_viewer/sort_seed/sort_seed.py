from .get_value_func import get_value_function
from multiprocessing import cpu_count
from concurrent.futures import ProcessPoolExecutor
from CApi import get_galaxy_data_c

def get_seed_value(args, value_func):
    galaxy_data = get_galaxy_data_c(*args)
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
