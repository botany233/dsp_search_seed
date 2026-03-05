import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from copy import deepcopy
from time import perf_counter, sleep

from CApi import *
from benchmark_condition import debug_condition_functions

cpu_thread = 20
gpu_thread = 4
device_id = 1
local_size = 256

set_device_id_c(device_id)
set_local_size_c(local_size)
set_gpu_max_worker_c(gpu_thread)

galaxy_condition = {"stars": [{"type": "蓝巨星", "satisfy_num": 3}]}

seeds = (0, 100000)
star_nums = (64, 64)
quick = False

galaxy_condition = change_galaxy_condition_legal(galaxy_condition)
check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, galaxy_condition, quick, cpu_thread)
check_batch_manager.run()
while check_batch_manager.is_running():
    sleep(0.1)
result = check_batch_manager.get_results()
result = sorted(result, key=lambda x: x.seed_id * 33 + x.star_num)
print(f"找到{len(result)}个种子")
for i in result:
    print(f"{i.seed_id}, {i.star_num}")
