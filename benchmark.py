from CApi import *
from functools import partial
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
from tqdm import tqdm
from math import ceil

# vein_names = "铁, 铜, 硅, 钛, 石, 煤, 油, 可燃冰, 金伯利, 分型硅, 有机晶体, 光栅石, 刺笋结晶, 单极磁石"
# liquid = "水, 硫酸"
# planet_types = ["地中海", "气态巨星", "冰巨星", "高产气巨", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩",
#                 "冰原冻土", "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩",
#                 "樱林海", "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]
# star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
#               "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星"]
# singularity = ["卫星", "多卫星", "潮汐锁定永昼永夜", "潮汐锁定1:2", "潮汐锁定1:4", "横躺自转", "反向自转"]

def check_seeds_py(seeds: tuple[int, int],
                   star_nums: tuple[int, int],
                   galaxy_condition: dict,
                   quick: bool,
                   batch_size: int,
                   max_thread: int,
                   record_seed: bool,
                   init_args: tuple[int, int]):
    max_thread = min(max_thread, cpu_count())
    with ProcessPoolExecutor(max_workers = max_thread, initializer=init_process, initargs=init_args) as executor:
        generator = batch_generator_py(galaxy_condition, quick, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper_py, generator)
        for result in results:
            if record_seed:
                with open("result_py.csv", "a") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

def check_seeds_c(seeds: tuple[int, int],
                  star_nums: tuple[int, int],
                  galaxy_str: str,
                  quick: bool,
                  batch_size: int,
                  max_thread: int,
                  record_seed: bool,
                  init_args: tuple[int, int]):
    max_thread = min(max_thread, cpu_count())
    with ProcessPoolExecutor(max_workers = max_thread, initializer=init_process, initargs=init_args) as executor:
        generator = batch_generator_c(galaxy_str, quick, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper_c, generator)
        for result in results:
            if record_seed:
                with open("result_c.csv", "a") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

if __name__ == "__main__":
    from time import perf_counter, sleep
    from benchmark_condition import *
    from sys import exit

    # galaxy_condition = debug_condition_100k_factory()
    # galaxy_condition = debug_condition_extreme_factory()
    # galaxy_condition = debug_condition_ttenyx()

    # galaxy_condition = benchmark_condition_100k_factory()
    # galaxy_condition = benchmark_condition_extreme_factory()
    # galaxy_condition = benchmark_condition_ttenyx_simple()
    # galaxy_condition = benchmark_condition_easy()
    galaxy_condition = benchmark_condition_3_blue()
    # galaxy_condition["veins_group"] = {"单极磁石": 24}

    galaxy_condition = change_galaxy_condition_legal(galaxy_condition)

    seeds = (0, 99999)
    star_nums = (64, 64)
    batch_size = 256
    max_thread = 20
    device_id = 0
    local_size = 256

    quick = 1
    record_seed = 0

    # debug_seed = 12
    # debug_star_num = 64
    # print(check_batch_py(debug_seed, debug_seed+1, debug_star_num, debug_star_num+1, galaxy_condition, bool(quick)))
    # print(check_batch_c(debug_seed, debug_seed+1, debug_star_num, debug_star_num+1, galaxy_condition, bool(quick)))
    # aaa

    # flag = perf_counter()
    # check_seeds_py(seeds, star_nums, galaxy_condition, bool(quick), batch_size, max_thread, record_seed, (device_id, local_size))
    # print(f"py多线程用时{perf_counter() - flag:.2f}s")

    flag = perf_counter()
    check_seeds_c(seeds, star_nums, galaxy_condition, bool(quick), batch_size, max_thread, record_seed, (device_id, local_size))
    print(f"c++多线程用时{perf_counter() - flag:.2f}s")

    flag = perf_counter()
    check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, galaxy_condition, bool(quick), max_thread)
    check_batch_manager.run()
    while check_batch_manager.is_running():
        sleep(0.01)
    result = check_batch_manager.get_results()
    result = sorted(result, key=lambda x: x.seed_id * 33 + x.star_num)
    if record_seed:
        with open("result_c_new.csv", "a") as f:
            f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", result))
    print(f"c++新多线程用时{perf_counter() - flag:.2f}s")
