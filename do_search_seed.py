from check_seed import check_seed
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count

# vein_names = "铁, 铜, 硅, 钛, 石, 煤, 油, 可燃冰, 金伯利, 分型硅, 有机晶体, 光栅石, 刺笋结晶, 单极磁石"
# liquid = "水, 硫酸"
# planet_types = ["地中海", "气态巨星", "冰巨星", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩", "冰原冻土",
#                 "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩", "樱林海",
#                 "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]


def check_batch(start_seed:int, end_seed:int, star_num: int, galaxy_condition:dict)->list[str]:
    result = []
    for seed in range(start_seed, end_seed):
        if check_seed(seed, star_num, galaxy_condition):
            result.append(seed)
    return result

def batch_generator(galaxy_condition:dict,
                    start_seed:int,
                    end_seed:int,
                    start_star_num: int,
                    end_star_num: int,
                    batch_size:int
                    ):
    for star_num in range(start_star_num, end_star_num+1):
        for seed in range(start_seed, end_seed+1, batch_size):
            yield seed, min(seed+batch_size, end_seed+1), star_num, galaxy_condition

def check_batch_wrapper(args):
    return check_batch(*args)

if __name__ == "__main__":
    from time import perf_counter
    planet_condition_1 = {"veins": {"金伯利":1, "刺笋结晶":1, "煤":1}, "type": "红石"}
    planet_condition_2 = {"veins": {"分型硅":1, "油":1, "刺笋结晶":1, "煤":1}, "type": "潘多拉沼泽"}
    planet_condition_3 = {"veins": {"光栅石":1}, "singularity": "卫星", "type": "橙晶荒漠"}

    star_condition_1 = {"planets": [planet_condition_1,]}
    star_condition_2 = {"planets": [planet_condition_2,]}
    star_condition_3 = {"planets": [planet_condition_3,]}

    galaxy_condition = {"veins": {"单极磁石":4}, "stars":[star_condition_1, star_condition_2, star_condition_3]}

    start_seed, end_seed = 0, 19999
    start_star_num, end_star_num = 64, 64
    batch_size = 128
    max_thread = 20

    flag = perf_counter()
    with ProcessPoolExecutor(max_workers = min(max_thread, cpu_count())) as executor:
        generator = batch_generator(galaxy_condition, start_seed, end_seed, start_star_num, end_star_num, batch_size)

        results = executor.map(check_batch_wrapper, generator)
        for result in results:
            with open("result.txt", "a") as f:
                f.writelines(map(lambda x: f"{x}\n", result))
    print(f"用时{perf_counter() - flag:.2f}s")

    flag = perf_counter()
    for seed in range(start_seed, end_seed+1):
        if check_seed(seed, 64, galaxy_condition):
            with open("result_single_thread.txt", "a") as f:
                f.write(f"{seed}\n")
    print(f"用时{perf_counter() - flag:.2f}s")
