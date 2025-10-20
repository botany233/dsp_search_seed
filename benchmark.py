from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
import json

# vein_names = "铁, 铜, 硅, 钛, 石, 煤, 油, 可燃冰, 金伯利, 分型硅, 有机晶体, 光栅石, 刺笋结晶, 单极磁石"
# liquid = "水, 硫酸"
# planet_types = ["地中海", "气态巨星", "冰巨星", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩", "冰原冻土",
#                 "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩", "樱林海",
#                 "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]
# star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
#               "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星", "X型恒星"]
# singularity = ["卫星", "多卫星", "潮汐锁定永昼永夜", "潮汐锁定1:2", "潮汐锁定1:4", "横躺自转", "反向自转"]

def batch_generator(galaxy_condition:dict|str,
                    seeds: tuple[int, int],
                    star_nums: tuple[int, int],
                    batch_size:int):
    for star_num in range(star_nums[0], star_nums[1]+1):
        for seed in range(seeds[0], seeds[1]+1, batch_size):
            yield seed, min(seed+batch_size, seeds[1]+1), star_num, galaxy_condition

def batch_generator_c(galaxy_str:str,
                      galaxy_str_no_veins:str,
                      seeds: tuple[int, int],
                      star_nums: tuple[int, int],
                      batch_size:int):
    for star_num in range(star_nums[0], star_nums[1]+1):
        for seed in range(seeds[0], seeds[1]+1, batch_size):
            yield seed, min(seed+batch_size, seeds[1]+1), star_num, galaxy_str, galaxy_str_no_veins

def check_batch_wrapper(args):
    return check_batch_c(*args)

def check_batch_wrapper_py(args):
    return check_batch_py(*args)

def check_seeds_py(seeds: tuple[int, int],
                           star_nums: tuple[int, int],
                           galaxy_condition: dict,
                           batch_size: int,
                           max_thread: int,
                           record_seed: bool):
    with ProcessPoolExecutor(max_workers = min(max_thread, cpu_count())) as executor:
        generator = batch_generator(galaxy_condition, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper_py, generator)
        for result in results:
            if record_seed:
                with open("result_py.txt", "a") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

def check_seeds_c(seeds: tuple[int, int],
                           star_nums: tuple[int, int],
                           galaxy_str: str,
                           galaxy_str_no_veins: str,
                           batch_size: int,
                           max_thread: int,
                           record_seed: bool):
    with ProcessPoolExecutor(max_workers = min(max_thread, cpu_count())) as executor:
        generator = batch_generator_c(galaxy_str, galaxy_str_no_veins, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper, generator)
        for result in results:
            if record_seed:
                with open("result_c.txt", "a") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

if __name__ == "__main__":
    from time import perf_counter
    planet_condition_1 = {"veins": {"金伯利":1, "刺笋结晶":1, "煤":1}, "type": "红石"}
    planet_condition_2 = {"veins": {"分型硅":1, "油":1, "刺笋结晶":1, "煤":1}, "type": "潘多拉沼泽"}
    planet_condition_3 = {"veins": {"光栅石":1}, "singularity": "卫星", "type": "橙晶荒漠"}
    planet_condition_4 = {"singularity": "潮汐锁定永昼永夜"}
    planet_condition_5 = {"type": "气态巨星"}
    planet_condition_6 = {"liquid": "水"}
    planet_condition_7 = {"liquid": "硫酸"}
    planet_condition_8 = {"is_in_dsp": True}

    star_condition_1 = {"planets": [planet_condition_1]}
    star_condition_2 = {"planets": [planet_condition_2]}
    star_condition_3 = {"planets": [planet_condition_3,]}
    star_condition_4 = {"planets": [planet_condition_4, planet_condition_5, planet_condition_6, planet_condition_7, planet_condition_8],
                        "veins": {"油":1, "可燃冰":1, "金伯利":1, "分型硅":1, "有机晶体":1, "光栅石":1, "刺笋结晶":1}}

    galaxy_condition = {"veins": {"单极磁石":4}, "stars":[star_condition_1, star_condition_2, star_condition_3, star_condition_4]}

    galaxy_condition = change_veins_to_legal(galaxy_condition)
    galaxy_condition_no_veins = get_galaxy_condition_no_veins(galaxy_condition)

    galaxy_str = json.dumps(galaxy_condition, ensure_ascii = False)
    galaxy_str_no_veins = json.dumps(galaxy_condition_no_veins, ensure_ascii = False)

    seeds = (0, 19999)
    star_nums = (32, 64)
    batch_size = 512
    max_thread = 20

    record_seed = 1

    # flag = perf_counter()
    # check_seeds_py(seeds, star_nums, galaxy_condition, batch_size, max_thread, record_seed)
    # print(f"py多线程用时{perf_counter() - flag:.2f}s")

    flag = perf_counter()
    check_seeds_c(seeds, star_nums, galaxy_str, galaxy_str_no_veins, batch_size, max_thread, record_seed)
    print(f"c++多线程用时{perf_counter() - flag:.2f}s")
