from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count

from .search_seed import *
from .const_values import *

def check_planet_py(planet_data: PlanetData, planet_condition: PlanetCondition) -> bool:
    if planet_condition.dsp_level > planet_data.dsp_level:
        return False
    if ((1 << (planet_data.type_id - 1)) & planet_condition.type) == 0:
        return False
    if (planet_condition.liquid & planet_data.liquid) != planet_condition.liquid:
        return False
    if (planet_condition.singularity & planet_data.singularity) != planet_condition.singularity:
        return False
    if planet_condition.need_veins:
        if any(planet_data.veins_group[i] < planet_condition.veins_group[i] for i in range(14)):
            return False
        if any(planet_data.veins_point[i] < planet_condition.veins_point[i] for i in range(14)):
            return False
    for moon_condition in planet_condition.moons:
        left_satisfy_num = moon_condition.satisfy_num
        for moon_data in planet_data.moons:
            if check_planet_py(moon_data, moon_condition):
                left_satisfy_num -= 1
                if not left_satisfy_num:
                    break
        if left_satisfy_num:
            return False
    return True

def check_star_py(star_data: StarData, star_condition: StarCondition) -> bool:
    if ((1 << (star_data.type_id - 1)) & star_condition.type) == 0:
        return False
    if star_condition.distance < star_data.distance:
        return False
    if star_condition.dyson_lumino > star_data.dyson_lumino:
        return False
    if star_condition.need_veins:
        if any(star_data.veins_group[i] < star_condition.veins_group[i] for i in range(14)):
            return False
        if any(star_data.veins_point[i] < star_condition.veins_point[i] for i in range(14)):
            return False
    for planet_condition in star_condition.planets:
        left_satisfy_num = planet_condition.satisfy_num
        for planet_data in star_data.planets:
            if check_planet_py(planet_data, planet_condition):
                left_satisfy_num -= 1
                if not left_satisfy_num:
                    break
        if left_satisfy_num:
            return False
    return True

def check_galaxy_py(galaxy_data: GalaxyData, galaxy_condition: GalaxyCondition) -> bool:
    if galaxy_condition.need_veins:
        if any(galaxy_data.veins_group[i] < galaxy_condition.veins_group[i] for i in range(14)):
            return False
        if any(galaxy_data.veins_point[i] < galaxy_condition.veins_point[i] for i in range(14)):
            return False
    for star_condition in galaxy_condition.stars:
        left_satisfy_num = star_condition.satisfy_num
        for star_data in galaxy_data.stars:
            if check_star_py(star_data, star_condition):
                left_satisfy_num -= 1
                if not left_satisfy_num:
                    break
        if left_satisfy_num:
            return False
    for planet_condition in galaxy_condition.planets:
        left_satisfy_num = planet_condition.satisfy_num
        for star_data in galaxy_data.stars:
            if not left_satisfy_num:
                break
            for planet_data in star_data.planets:
                if check_planet_py(planet_data, planet_condition):
                    left_satisfy_num -= 1
                    if not left_satisfy_num:
                        break
        if left_satisfy_num:
            return False
    return True

def check_seed_py(seed: int, star_num: int, resource_rate: float, galaxy_condition: GalaxyCondition, quick: bool) -> bool:
    if not quick:
        galaxy_data = get_galaxy_data_c(Seed(seed, star_num, resource_rate), True)
        if not check_galaxy_py(galaxy_data, galaxy_condition):
            return False
    galaxy_data = get_galaxy_data_c(Seed(seed, star_num, resource_rate), quick)
    return check_galaxy_py(galaxy_data, galaxy_condition)

def check_batch_py(tasks: list[tuple[int, int, float]], galaxy_condition: dict, quick: bool) -> list[tuple[int, int]]:
    galaxy_condition = galaxy_condition_to_struct(galaxy_condition)
    result = []
    for seed, star_num, resource_rate in tasks:
        if check_seed_py(seed, star_num, resource_rate, galaxy_condition, quick):
            result.append((seed, star_num))
    return result

def init_process(device_id: int, local_size: int):
    do_init_c()
    if not set_device_id_c(device_id):
        print("Set device id failed! Roll back to cpu!")
    set_local_size_c(local_size)

def get_task_seed_wrapper(seeds: tuple[int, int], star_nums: tuple[int, int], resource_rate: float) -> callable:
    def get_task_seed(task_id: int) -> tuple[int, int, float]:
        star_num = task_id % (star_nums[1] - star_nums[0] + 1) + star_nums[0]
        seed = task_id // (star_nums[1] - star_nums[0] + 1) + seeds[0]
        return seed, star_num, resource_rate
    return get_task_seed

# def check_seeds_py(seeds: tuple[int, int],
#                    star_nums: tuple[int, int],
#                    galaxy_condition: dict,
#                    quick: bool,
#                    max_thread: int,
#                    device_id: int,
#                    local_size: int) -> list[tuple[int, int]]:
#     get_task_seed = get_task_seed_wrapper(seeds, star_nums)
#     task_num = (seeds[1] - seeds[0] + 1) * (star_nums[1] - star_nums[0] + 1)
#     tasks = [get_task_seed(i) for i in range(task_num)]
#     results = check_batch_py(tasks, galaxy_condition, quick)
#     return results

def check_seeds_py(seeds: tuple[int, int],
                   star_nums: tuple[int, int],
                   resource_rate: float,
                   galaxy_condition: dict,
                   quick: bool,
                   max_thread: int,
                   device_id: int,
                   local_size: int) -> list[tuple[int, int]]:
    get_task_seed = get_task_seed_wrapper(seeds, star_nums, resource_rate)
    max_thread = min(max_thread, cpu_count())
    task_num = (seeds[1] - seeds[0] + 1) * (star_nums[1] - star_nums[0] + 1)
    batch_size = min(task_num // (max_thread * 20), 1024)
    with ProcessPoolExecutor(max_workers = max_thread, initializer=init_process, initargs=(device_id, local_size)) as executor:
        futures = []
        for task_id in range(0, task_num, batch_size):
            tasks = [get_task_seed(i) for i in range(task_id, min(task_id + batch_size, task_num))]
            futures.append(executor.submit(check_batch_py, tasks, galaxy_condition, quick))

        results = []
        for future in futures:
            result = future.result()
            results.extend(result)
    return results

__all__ = ["check_seed_py", "check_seeds_py"]
