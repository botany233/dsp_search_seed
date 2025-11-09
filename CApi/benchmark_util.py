from .check_seed import check_batch_py
from .search_seed import check_batch_c

def batch_generator_py(galaxy_condition:dict,
                    seeds: tuple[int, int],
                    star_nums: tuple[int, int],
                    batch_size:int):
    for seed in range(seeds[0], seeds[1]+1, batch_size):
        yield seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_condition

def batch_generator_c(galaxy_str_1:str,
                      galaxy_str_2:str,
                      galaxy_str_3:str,
                      seeds: tuple[int, int],
                      star_nums: tuple[int, int],
                      batch_size:int):
    for seed in range(seeds[0], seeds[1]+1, batch_size):
        yield seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_str_1, galaxy_str_2, galaxy_str_3

def check_batch_wrapper_c(args):
    return check_batch_c(*args)

def check_batch_wrapper_py(args):
    return check_batch_py(*args)
