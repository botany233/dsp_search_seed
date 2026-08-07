import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from time import perf_counter, sleep

from CApi import *

galaxy_condition_raw = {
    "condition": {
        "valid_state": True,
        "stars": [
            {
                "satisfy_num": 3,
                "dyson_lumino": 0.9941314210289814,
                "planets": [
                    {
                        "satisfy_num": 1,
                        "dsp_level": "全接收",
                        "type": [
                            "红石",
                            "橙晶荒漠",
                            "灰烬冻土",
                            "潘多拉沼泽",
                            "飓风石林",
                            "干旱荒漠",
                            "冰原冻土",
                            "冰巨星",
                            "黑石盐滩",
                            "戈壁",
                            "气态巨星",
                            "极寒冻土"
                        ]
                    }
                ]
            }
        ],
        "planets": [
            {
                "satisfy_num": 3,
                "liquid": "硫酸"
            },
            {
                "satisfy_num": 2,
                "type": [
                    "气态巨星",
                    "极寒冻土",
                    "水世界",
                    "高产气巨",
                    "草原",
                    "海洋丛林",
                    "樱林海",
                    "冰巨星",
                    "黑石盐滩",
                    "灰烬冻土",
                    "戈壁",
                    "地中海"
                ]
            },
            {
                "satisfy_num": 1,
                "dsp_level": "全接收",
                "type": [
                    "火山灰",
                    "潘多拉沼泽",
                    "草原",
                    "红石",
                    "干旱荒漠",
                    "熔岩",
                    "冰巨星",
                    "冰原冻土",
                    "极寒冻土",
                    "地中海",
                    "樱林海"
                ]
            }
        ]
    },
    "seeds": [
        0,
        199999
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
    gpu_thread = 8
    device_id = -1
    local_size = 256

    c_save_path = os.path.join(os.path.dirname(__file__), "debug_results_c.csv")
    py_save_path = os.path.join(os.path.dirname(__file__), "debug_results_py.csv")
    differ_save_path = os.path.join(os.path.dirname(__file__), "debug_results_differ.csv")
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

    py_results_set = set(py_results)
    c_results_set = set((result.seed_id, result.star_num) for result in c_results)

    py_new_result = sorted(py_results_set - c_results_set, key=lambda x: x[0] * 33 + x[1])
    c_new_result = sorted(c_results_set - py_results_set, key=lambda x: x[0] * 33 + x[1])
    if len(py_new_result) > 0 or len(c_new_result) > 0:
        print(f"differ num = {len(py_new_result) + len(c_new_result)}")
        with open(differ_save_path, "w") as f:
            f.write("py unique results:\n")
            f.writelines(map(lambda x: f"{x[0]}, {x[1]}\n", py_new_result))
            f.write("cpp unique results:\n")
            f.writelines(map(lambda x: f"{x[0]}, {x[1]}\n", c_new_result))

if __name__ == "__main__":
    main()
