import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from time import perf_counter, sleep

from CApi import *

galaxy_condition_raw = {
    "condition": {
        "stars":[
            {"distance": 10, "type": "蓝巨星", "satisfy_num": 1},
            {"distance": 15, "type": "O型恒星", "satisfy_num": 2, "dyson_lumino": 2.4}
        ]
    },
    "seeds": [
        5092,
        5092
    ],
    "star_nums": [
        64,
        64
    ],
    "resource_index": 10,
    "quick": True
}

def main():
    cpu_thread = 20
    gpu_thread = 4
    device_id = 0
    local_size = 256

    c_save_path = os.path.join(os.path.dirname(__file__), "debug_results_c.csv")
    py_save_path = os.path.join(os.path.dirname(__file__), "debug_results_py.csv")
    set_device_id_c(device_id)
    set_local_size_c(local_size)
    set_gpu_max_worker_c(gpu_thread)

    galaxy_condition = galaxy_condition_raw["condition"]
    seeds = galaxy_condition_raw["seeds"]
    star_nums = galaxy_condition_raw["star_nums"]
    resource_index = galaxy_condition_raw["resource_index"]
    quick = galaxy_condition_raw["quick"]

    galaxy_condition = change_galaxy_condition_legal(galaxy_condition)

    flag = perf_counter()
    py_results = check_seeds_py(seeds, star_nums, resource_index, galaxy_condition, quick, cpu_thread, device_id, local_size)
    with open(py_save_path, "w") as f:
        f.writelines(map(lambda x: f"{x[0]}, {x[1]}\n", py_results))
    print(f"py完成: 用时{perf_counter() - flag:.2f}s，找到{len(py_results)}个种子")

    flag = perf_counter()
    check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, resource_index, galaxy_condition, quick, cpu_thread)
    check_batch_manager.run()
    while check_batch_manager.is_running():
        sleep(0.1)
    c_results = check_batch_manager.get_results()
    c_results = sorted(c_results, key=lambda x: x.seed_id * 33 + x.star_num)
    with open(c_save_path, "w") as f:
        f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", c_results))
    print(f"cpp完成: 用时{perf_counter() - flag:.2f}s，找到{len(c_results)}个种子")

if __name__ == "__main__":
    main()
