import os
cur_dir = os.path.dirname(__file__)
import sys
import json
sys.path.append(os.path.dirname(cur_dir))
from copy import deepcopy
from time import perf_counter, sleep

from CApi import *
from debug_random_condition import get_random_debug_condition

def main(test_num: int, load_condition: bool = False):
    cpu_thread = 20
    gpu_thread = 8
    device_id = 1
    local_size = 256

    set_device_id_c(device_id)
    set_local_size_c(local_size)
    set_gpu_max_worker_c(gpu_thread)

    for test_index in range(test_num):
        if load_condition:
            with open(os.path.join(cur_dir, f"debug_random_condition.json"), "r", encoding="utf-8") as f:
                condition_data = json.load(f)
                galaxy_condition_raw = condition_data["condition"]
                seeds = condition_data["seeds"]
                star_nums = condition_data["star_nums"]
                resource_index = condition_data["resource_index"]
                quick = condition_data["quick"]
        else:
            galaxy_condition_raw, seeds, star_nums, resource_index, quick = get_random_debug_condition()
        galaxy_condition = change_galaxy_condition_legal(deepcopy(galaxy_condition_raw))

        flag = perf_counter()
        py_results = check_seeds_py(seeds, star_nums, resource_index, galaxy_condition, quick, cpu_thread, device_id, local_size)
        py_time = perf_counter() - flag

        flag = perf_counter()
        check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, resource_index, galaxy_condition, quick, cpu_thread)
        check_batch_manager.run()
        task_num = check_batch_manager.get_task_num()
        while check_batch_manager.is_running():
            print(f"当前进度{check_batch_manager.get_task_progress()+1}/{task_num}", end="\r")
            sleep(0.1)
        print(" " * 50, end="\r")
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
    main(1000, os.path.exists(os.path.join(cur_dir, "debug_random_condition.json")))
