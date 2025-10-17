from check_seed import check_seed
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count

# vein_names = "铁, 铜, 硅, 钛, 石, 煤, 油, 可燃冰, 金伯利, 分型硅, 有机晶体, 光栅石, 刺笋结晶, 单极磁石"
# liquid = "水, 硫酸"
# planet_types = ["地中海", "气态巨星", "冰巨星", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩", "冰原冻土",
#                 "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩", "樱林海",
#                 "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]
# star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
#               "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星", "X型恒星"]
# singularity = ["卫星", "多卫星", "潮汐锁定永昼永夜", "潮汐锁定1:2", "潮汐锁定1:4", "横躺自转", "反向自转"]

def check_batch(start_seed:int, end_seed:int, star_num: int, galaxy_condition:dict)->list[str]:
    result = []
    for seed in range(start_seed, end_seed):
        if check_seed(seed, star_num, galaxy_condition):
            result.append(f"{star_num} {seed}")
    return result

def batch_generator(galaxy_condition:dict,
                    seeds: tuple[int, int],
                    star_nums: tuple[int, int],
                    batch_size:int
                    ):
    for star_num in range(star_nums[0], star_nums[1]+1):
        for seed in range(seeds[0], seeds[1]+1, batch_size):
            yield seed, min(seed+batch_size, seeds[1]+1), star_num, galaxy_condition

def check_batch_wrapper(args):
    return check_batch(*args)

def check_seed_single_thread(seeds: tuple[int, int],
                             star_nums: tuple[int, int],
                             galaxy_condition: dict
                             ):
    generator = batch_generator(galaxy_condition, seeds, star_nums, 1)
    for seed, _, star_num, galaxy_condition in generator:
        if check_seed(seed, star_num, galaxy_condition):
            with open("result_single_thread.txt", "a") as f:
                f.write(f"{star_num} {seed}\n")

def check_seed_mult_thread(seeds: tuple[int, int],
                           star_nums: tuple[int, int],
                           galaxy_condition: dict,
                           batch_size: int,
                           max_thread: int
                           ):
    with ProcessPoolExecutor(max_workers = min(max_thread, cpu_count())) as executor:
        generator = batch_generator(galaxy_condition, seeds, star_nums, batch_size)

        results = executor.map(check_batch_wrapper, generator)
        for result in results:
            with open("result_mult_thread.txt", "a") as f:
                f.writelines(map(lambda x: f"{x}\n", result))

if __name__ == "__main__":
    from time import perf_counter
    planet_condition_1 = {"veins": {"金伯利":1, "刺笋结晶":1, "煤":1}, "type": "红石"}
    planet_condition_2 = {"veins": {"分型硅":1, "油":1, "刺笋结晶":1, "煤":1}, "type": "潘多拉沼泽"}
    planet_condition_3 = {"veins": {"光栅石":1}, "singularity": "卫星", "type": "橙晶荒漠"}

    star_condition_1 = {"planets": [planet_condition_1,]}
    star_condition_2 = {"planets": [planet_condition_2,]}
    star_condition_3 = {"planets": [planet_condition_3,]}

    galaxy_condition = {"veins": {"单极磁石":4}, "stars":[star_condition_1, star_condition_2, star_condition_3]}

    seeds = (0, 999)
    star_nums = (32, 64)
    batch_size = 128
    max_thread = 20

    flag = perf_counter()
    check_seed_single_thread(seeds, star_nums, galaxy_condition)
    print(f"用时{perf_counter() - flag:.2f}s")

    flag = perf_counter()
    check_seed_mult_thread(seeds, star_nums, galaxy_condition, batch_size, max_thread)
    print(f"用时{perf_counter() - flag:.2f}s")
