from .search_seed import *
from .check_seed import *

def batch_generator_py(galaxy_condition: dict,
                       quick: bool,
                       seeds: tuple[int, int],
                       star_nums: tuple[int, int],
                       batch_size:int):
    for seed in range(seeds[0], seeds[1]+1, batch_size):
        yield seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_condition, quick

def batch_generator_c(galaxy_str: str,
                      quick: bool,
                      seeds: tuple[int, int],
                      star_nums: tuple[int, int],
                      batch_size:int):
    for seed in range(seeds[0], seeds[1]+1, batch_size):
        yield seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_str, quick

def check_batch_py(start_seed:int, end_seed:int, start_star_num: int, end_star_num: int, galaxy_condition:dict, quick: bool)->list[str]:
    galaxy_condition = galaxy_condition_to_struct(galaxy_condition)

    result = []
    for seed in range(start_seed, end_seed):
        for star_num in range(start_star_num, end_star_num):
            if check_seed_py(seed, star_num, galaxy_condition, quick):
                result.append(f"{seed}, {star_num}")
    return result

def check_batch_wrapper_py(args):
    return check_batch_py(*args)

def check_batch_wrapper_c(args):
    return check_batch_c(*args)
