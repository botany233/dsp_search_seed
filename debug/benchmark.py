import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from time import perf_counter, sleep

from CApi import *
from benchmark_condition import benchmark_condition_functions

cpu_thread = 20

set_device_id_c(-1)
# set_local_size_c(256)
# set_gpu_max_worker_c(4)

galaxy_conditions = []
for condition_func in benchmark_condition_functions:
    galaxy_condition, seeds, star_nums, quick = condition_func()
    galaxy_condition = change_condition_legal(galaxy_condition)
    name = condition_func.__name__
    if quick:
        if not name.endswith("_quick"):
            name += "_quick"
        galaxy_conditions.append((name, galaxy_condition, seeds, star_nums, True))
    else:
        if not name.endswith("_standard"):
            name += "_standard"
        galaxy_conditions.append((name, galaxy_condition, seeds, star_nums, False))

benchmark_results = {name: [] for name, *_ in galaxy_conditions}
for round_index in range(4):
    print(f"第{round_index + 1}轮测试" + ("（预热，不计入成绩）" if round_index == 0 else ""))
    for name, galaxy_condition, seeds, star_nums, quick in galaxy_conditions:
        flag = perf_counter()
        check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, 10, galaxy_condition, quick, cpu_thread)
        check_batch_manager.run()
        while check_batch_manager.is_running():
            sleep(0.05)
        result_count = check_batch_manager.get_result_num()
        flag = perf_counter() - flag
        speed = (seeds[1]-seeds[0]+1)*(star_nums[1]-star_nums[0]+1) / flag
        results = benchmark_results[name]
        if results and result_count != results[0][2]:
            raise RuntimeError(f"{name} 第{round_index + 1}轮种子数量不一致：预期{results[0][2]}，实际{result_count}")
        results.append((flag, speed, result_count))
        print(f"{name} 完成，用时{flag:.2f}s，找到{result_count}个种子，速度{speed:.3f}seed/s")

print("最终结果（第2~4轮均值）")
for name, *_ in galaxy_conditions:
    results = benchmark_results[name][1:]
    average_time = sum(result[0] for result in results) / len(results)
    average_speed = sum(result[1] for result in results) / len(results)
    print(f"{name} 完成，平均用时{average_time:.2f}s，找到{results[0][2]}个种子，平均速度{average_speed:.3f}seed/s")
