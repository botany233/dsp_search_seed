from concurrent.futures import ProcessPoolExecutor
from math import sqrt
from multiprocessing import cpu_count

from .search_seed import (
    GalaxyData, PlanetData, Seed, StarData,
    do_init_c, get_galaxy_data_fast_c, set_device_id_c,
)
from .max_flow import MaxFlowGraph


def check_mineral_py(data: GalaxyData | StarData | PlanetData, condition: dict) -> bool:
    return all(data.veins_point[i] >= condition["veins_point"][i] and data.veins_amount[i] >= condition["veins_amount"][i] for i in range(14))


def check_count_py(candidates, condition: dict, check) -> bool:
    need = condition["satisfy_num"]
    if need <= 0:
        return True
    for candidate in candidates:
        if check(candidate, condition):
            need -= 1
            if need <= 0:
                return True
    return False


def check_planet_py(planet_data: PlanetData, planet_condition: dict) -> bool:
    if planet_condition["dsp_level"] > planet_data.dsp_level:
        return False
    if ((1 << planet_data.type_id) & planet_condition["type"]) == 0:
        return False
    liquid_masks = (0xFFFFFFFF, 0x00025C51, 0x00000200)
    if ((1 << planet_data.type_id) & liquid_masks[planet_condition["liquid"]]) == 0:
        return False
    singularity = planet_condition["singularity"]
    if (singularity & planet_data.singularity) != singularity:
        return False
    if not check_mineral_py(planet_data, planet_condition):
        return False
    return all(check_count_py(planet_data.moons, moon, check_planet_py) for moon in planet_condition["moons"])


def check_star_py(star_data: StarData, star_condition: dict) -> bool:
    if ((1 << star_data.type_id) & star_condition["type"]) == 0:
        return False
    # C++ 将这两个条件字段转换为 float。
    if star_condition["distance"] < star_data.distance:
        return False
    if star_condition["dyson_lumino"] > star_data.dyson_lumino:
        return False
    if not check_mineral_py(star_data, star_condition):
        return False
    return all(check_count_py(star_data.planets, planet, check_planet_py) for planet in star_condition["planets"])


def get_bond_positions_py(galaxy_data: GalaxyData, condition: dict) -> list[list[float]]:
    positions = []
    if condition["type"] == 3:
        for star_data in galaxy_data.stars:
            for planet_data in star_data.planets:
                if check_planet_py(planet_data, condition["value"]):
                    positions.append(star_data.pos_ly)
    elif condition["type"] == 2:
        for star_data in galaxy_data.stars:
            if check_star_py(star_data, condition["value"]):
                positions.append(star_data.pos_ly)
    else:
        raise ValueError("绑定端点必须为恒星或行星条件")
    return positions


def check_bond_position_py(pos1: list[list[float]], pos2: list[list[float]], bond_condition: dict) -> bool:
    need = bond_condition["satisfy_num"]
    if need <= 0:
        return True
    limit1 = bond_condition["con1"]["value"]["satisfy_num"]
    limit2 = bond_condition["con2"]["value"]["satisfy_num"]
    if limit1 <= 0 or limit2 <= 0 or len(pos1) * limit1 < need or len(pos2) * limit2 < need:
        return False

    left_num = len(pos1)
    right_num = len(pos2)
    node_num = left_num + right_num + 2
    source = 0
    target = node_num - 1
    graph = MaxFlowGraph(node_num)

    for left_index in range(left_num):
        graph.add_edge(source, left_index + 1, limit1)
    for right_index in range(right_num):
        graph.add_edge(left_num + right_index + 1, target, limit2)

    for left_index, left_pos in enumerate(pos1):
        for right_index, right_pos in enumerate(pos2):
            if sqrt(sum((a - b) ** 2 for a, b in zip(left_pos, right_pos))) <= bond_condition["distance"]:
                graph.add_edge(left_index + 1, left_num + right_index + 1)
    return graph.flow(source, target, need)


def check_galaxy_py(galaxy_data: GalaxyData, galaxy_condition: dict) -> bool:
    condition = galaxy_condition["value"]
    match galaxy_condition["type"]:
        case 1:
            return check_mineral_py(galaxy_data, condition)
        case 2:
            return check_count_py(galaxy_data.stars, condition, check_star_py)
        case 3:
            planets = (planet for star in galaxy_data.stars for planet in star.planets)
            return check_count_py(planets, condition, check_planet_py)
        case 4:
            if condition["satisfy_num"] <= 0:
                return True
            pos1 = get_bond_positions_py(galaxy_data, condition["con1"])
            pos2 = get_bond_positions_py(galaxy_data, condition["con2"])
            return check_bond_position_py(pos1, pos2, condition)
        case 5:
            children = condition["children"]
            need = condition["satisfy_num"]
            if need == 0:
                return not check_galaxy_py(galaxy_data, children[0])
            if need > len(children):
                return False
            for index, child in enumerate(children):
                if check_galaxy_py(galaxy_data, child):
                    need -= 1
                    if need == 0:
                        return True
                if len(children) - index - 1 < need:
                    return False
            return False
        case _:
            raise ValueError(f"不支持的条件类型: {galaxy_condition['type']}")


def check_seed_py(seed: Seed, galaxy_condition: dict, quick: bool) -> bool:
    """检查经过 change_condition_legal 处理的条件字典，不修改输入。"""
    # 标准模式必须用真实资源求值，不能用快速上界的布尔结果否决 NOT 等逻辑。
    galaxy_data = get_galaxy_data_fast_c(seed, quick)
    return check_galaxy_py(galaxy_data, galaxy_condition)


def check_batch_py(tasks: list[tuple[int, int, int]], galaxy_condition: dict, quick: bool) -> list[tuple[int, int]]:
    result = []
    for seed_id, star_num, resource_index in tasks:
        if check_seed_py(Seed(seed_id, star_num, resource_index), galaxy_condition, quick):
            result.append((seed_id, star_num))
    return result


def init_process():
    do_init_c()
    set_device_id_c(-1)

def get_task_seed_wrapper(seeds: tuple[int, int], star_nums: tuple[int, int], resource_index: int) -> callable:
    def get_task_seed(task_id: int) -> tuple[int, int, int]:
        star_num = task_id % (star_nums[1] - star_nums[0] + 1) + star_nums[0]
        seed = task_id // (star_nums[1] - star_nums[0] + 1) + seeds[0]
        return seed, star_num, resource_index
    return get_task_seed

def check_seeds_py(seeds: tuple[int, int],
                   star_nums: tuple[int, int],
                   resource_index: int,
                   galaxy_condition: dict,
                   quick: bool,
                   max_thread: int) -> list[tuple[int, int]]:
    get_task_seed = get_task_seed_wrapper(seeds, star_nums, resource_index)
    max_thread = min(max_thread, cpu_count())
    task_num = (seeds[1] - seeds[0] + 1) * (star_nums[1] - star_nums[0] + 1)
    batch_size = max(1, min(task_num // (max_thread * 20), 1024))
    with ProcessPoolExecutor(max_workers = max_thread, initializer=init_process) as executor:
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
