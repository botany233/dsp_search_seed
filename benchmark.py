from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
import json

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
                   batch_size: int,
                   max_thread: int,
                   record_seed: bool):
    with ProcessPoolExecutor(max_workers = min(max_thread, cpu_count())) as executor:
        generator = batch_generator_py(galaxy_condition, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper_py, generator)
        for result in results:
            if record_seed:
                with open("result_py.csv", "a") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

def check_seeds_c(seeds: tuple[int, int],
                  star_nums: tuple[int, int],
                  galaxy_str: str,
                  galaxy_str_simple: str,
                  batch_size: int,
                  max_thread: int,
                  record_seed: bool):
    with ProcessPoolExecutor(max_workers = min(max_thread, cpu_count())) as executor:
        generator = batch_generator_c(galaxy_str, galaxy_str_simple, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper, generator)
        for result in results:
            if record_seed:
                with open("result_c.csv", "a") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

if __name__ == "__main__":
    from time import perf_counter
    from benchmark_condition import *
    from sys import exit

    # galaxy_condition = get_100k_factory_condition()
    # galaxy_condition = get_extreme_factory_condition()
    # galaxy_condition = get_easy_condition()
    galaxy_condition = get_3_blue_condition()
    # galaxy_condition["veins"] = {"单极磁石": 24}

    galaxy_condition = change_galaxy_condition_legal(galaxy_condition)
    galaxy_condition_simple = get_galaxy_condition_simple(galaxy_condition)

    galaxy_str = json.dumps(galaxy_condition, ensure_ascii = False)
    galaxy_str_simple = json.dumps(galaxy_condition_simple, ensure_ascii = False)

    print_condition_str = 0
    if print_condition_str:
        print(galaxy_str)
        print(galaxy_str_simple)
        exit()

    seeds = (0, 9999999)
    star_nums = (64, 64)
    batch_size = 1024
    max_thread = 20

    record_seed = 1

    # flag = perf_counter()
    # check_seeds_py(seeds, star_nums, galaxy_condition, batch_size, max_thread, record_seed)
    # print(f"py多线程用时{perf_counter() - flag:.2f}s")

    flag = perf_counter()
    check_seeds_c(seeds, star_nums, galaxy_str, galaxy_str_simple, batch_size, max_thread, record_seed)
    print(f"c++多线程用时{perf_counter() - flag:.2f}s")
