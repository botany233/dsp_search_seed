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
    name = condition_func.__name__
    if quick:
        if not name.endswith("_quick"):
            name += "_quick"
        galaxy_conditions.append((name, galaxy_condition, seeds, star_nums, True))
    else:
        if not name.endswith("_standard"):
            name += "_standard"
        galaxy_conditions.append((name, galaxy_condition, seeds, star_nums, False))

for name, galaxy_condition, seeds, star_nums, quick in galaxy_conditions:
    galaxy_condition = change_condition_legal(galaxy_condition)
    flag = perf_counter()
    check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, 10, galaxy_condition, quick, cpu_thread)
    check_batch_manager.run()
    while check_batch_manager.is_running():
        sleep(0.1)
    result = check_batch_manager.get_results()
    result = sorted(result, key=lambda x: x.seed_id * 33 + x.star_num)
    flag = perf_counter() - flag
    print(f"{name} 完成，用时{flag:.2f}s，找到{len(result)}个种子，速度{(seeds[1]-seeds[0]+1)*(star_nums[1]-star_nums[0]+1) / flag:.3f}seed/s")
