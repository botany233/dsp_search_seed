from .search_seed import check_batch_c

def batch_generator_c(galaxy_str: str,
                      quick_check: bool,
                      seeds: tuple[int, int],
                      star_nums: tuple[int, int],
                      batch_size:int):
    for seed in range(seeds[0], seeds[1]+1, batch_size):
        yield seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_str, quick_check

def check_batch_wrapper_c(args):
    return check_batch_c(*args)
