from time import perf_counter, sleep
from CApi import *
import os
from copy import deepcopy
from benchmark_condition import debug_condition_functions

max_thread = 19
device_id = 1
local_size = 256

if os.path.exists("debug_results.csv"):
    os.remove("debug_results.csv")
set_device_id_c(device_id)
set_local_size_c(local_size)

galaxy_conditions = []
for condition_func in debug_condition_functions:
    galaxy_condition, seeds, star_nums, quick = condition_func()
    if not quick:
        galaxy_conditions.append((f"{condition_func.__name__}_quick", deepcopy(galaxy_condition), seeds, star_nums, True))
    galaxy_conditions.append((f"{condition_func.__name__}_standard", galaxy_condition, seeds, star_nums, False))

for name, galaxy_condition, seeds, star_nums, quick in galaxy_conditions:
    galaxy_condition = change_galaxy_condition_legal(galaxy_condition)
    flag = perf_counter()
    check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, galaxy_condition, quick, max_thread)
    check_batch_manager.run()
    while check_batch_manager.is_running():
        sleep(0.1)
    result = check_batch_manager.get_results()
    result = sorted(result, key=lambda x: x.seed_id * 33 + x.star_num)
    flag = perf_counter() - flag
    with open("debug_results.csv", "a") as f:
        f.write(f"{name}:\n")
        f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", result))
    print(f"{name} 完成，用时{flag:.2f}s，找到{len(result)}个种子")
