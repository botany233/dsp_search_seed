import os
import sys
import json
import random
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from copy import deepcopy
from time import perf_counter, sleep

from CApi import *
from debug_random_condition import get_random_debug_condition

cur_dir = os.path.dirname(__file__)

def prepare_random_search(cpu_thread: int):
    # 每轮先选模式，防止预筛把标准矿物条件全部挤掉。
    need_veins, quick = random.choice([(False, True), (True, True), (True, False)])
    for attempt in range(1, 101):
        raw, seeds, star_nums, resource_index, quick = get_random_debug_condition(need_veins, quick)
        condition = change_galaxy_condition_legal(deepcopy(raw))
        flag = perf_counter()
        results = check_seeds_py(seeds, star_nums, resource_index, condition, quick, cpu_thread)
        elapsed = perf_counter() - flag
        # 仅用Python参考实现预筛，不依赖待校验的C++搜索结果。
        sample_num = seeds[1] - seeds[0] + 1
        if 0.05 <= len(results) / sample_num <= 0.6:
            print(f"预筛第{attempt}次选中条件，样本命中{len(results)}/{sample_num}")
            break
    else:
        raise RuntimeError("连续100个条件未获得10%~90%的样本命中率，请检查条件生成范围")

    # 预筛结果计入本轮搜索。每批目标5秒，逐批根据实测速度调整。
    py_time = elapsed
    end_seed = seeds[1]
    batch_size = sample_num
    while py_time < 40 and end_seed < 99999999:
        batch_seconds = min(5, 45 - py_time)
        batch_size = max(1, min(int(batch_size * batch_seconds / max(elapsed, 0.001)),
                                batch_size * 4, 100000, 99999999 - end_seed))
        batch_seeds = (end_seed + 1, end_seed + batch_size)
        flag = perf_counter()
        results.extend(check_seeds_py(batch_seeds, star_nums, resource_index, condition, quick, cpu_thread))
        elapsed = perf_counter() - flag
        py_time += elapsed
        end_seed = batch_seeds[1]
    return raw, (seeds[0], end_seed), star_nums, resource_index, quick, results, py_time


def main(test_num: int, load_condition: bool = False):
    cpu_thread = 20
    set_device_id_c(-1) #现在搜索已经不需要gpu了，直接禁用

    for test_index in range(test_num):
        if load_condition:
            with open(os.path.join(cur_dir, f"debug_random_condition.json"), "r", encoding="utf-8") as f:
                condition_data = json.load(f)
                galaxy_condition_raw = condition_data["condition"]
                seeds = condition_data["seeds"]
                star_nums = condition_data["star_nums"]
                resource_index = condition_data["resource_index"]
                quick = condition_data["quick"]
            galaxy_condition = change_galaxy_condition_legal(deepcopy(galaxy_condition_raw))
            flag = perf_counter()
            py_results = check_seeds_py(seeds, star_nums, resource_index, galaxy_condition, quick, cpu_thread)
            py_time = perf_counter() - flag
        else:
            galaxy_condition_raw, seeds, star_nums, resource_index, quick, py_results, py_time = prepare_random_search(cpu_thread)
            galaxy_condition = change_galaxy_condition_legal(deepcopy(galaxy_condition_raw))
        total = seeds[1] - seeds[0] + 1
        print(f"搜索范围: {seeds}，64星，{resource_rate_c[resource_index]}，{'快速' if quick else '标准'}，"
              f"Python命中率{len(py_results) / total:.2%}")
        if not 30 <= py_time <= 60:
            print(f"提示：本轮Python用时{py_time:.2f}s，超出30~60s目标范围")

        flag = perf_counter()
        check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, resource_index, galaxy_condition, quick, cpu_thread)
        check_batch_manager.run()
        task_num = check_batch_manager.get_task_num()
        while check_batch_manager.is_running():
            sleep(0.1)
        c_results = [(i.seed_id, i.star_num) for i in check_batch_manager.get_results()]
        c_results = sorted(c_results, key=lambda x: x[0] * 33 + x[1])
        c_time = perf_counter() - flag

        if py_results == c_results:
            print(f"测试{test_index+1}通过！py用时{py_time:.2f}s, c用时{c_time:.2f}s, 找到{len(py_results)}个种子")
            # print(galaxy_condition_raw)
        else:
            print(f"测试{test_index+1}失败！py用时{py_time:.2f}s, c用时{c_time:.2f}s")
            print(f"py找到{len(py_results)}个种子, c找到{len(c_results)}个种子")
            print(f"搜索范围: {seeds}-{star_nums}-{resource_rate_c[resource_index]}-{"快速" if quick else "标准"}")
            with open(os.path.join(cur_dir, f"debug_random_condition.json"), "w", encoding="utf-8") as f:
                json.dump({
                    "condition": galaxy_condition_raw,
                    "seeds": seeds,
                    "star_nums": star_nums,
                    "resource_index": resource_index,
                    "quick": quick
                }, f, ensure_ascii=False, indent=4)
            break

        if load_condition:
            break

if __name__ == "__main__":
    # 小规模测试10次就行，大规模一般1000
    main(10, os.path.exists(os.path.join(cur_dir, "debug_random_condition.json")))
