import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
import random
from time import perf_counter, sleep

from CApi import *

cpu_thread = 20
gpu_thread = 2
device_id = -1
local_size = 256
quick = False

set_device_id_c(device_id)
set_local_size_c(local_size)
set_gpu_max_worker_c(gpu_thread)

tasks = []
for i in range(500):
    seed_id = random.randint(0, 99999999)
    resource_index = random.randint(0, 10)
    tasks.append((seed_id, 64, resource_index))
dummy_results = []

get_data_manager = GetDataManager(cpu_thread, quick, local_size)

tag = perf_counter()
for seed_id, star_num, resource_index in tasks:
    get_data_manager.add_task(seed_id, star_num, resource_index)

finish_num, task_num = 0, len(tasks)
while finish_num < task_num:
    results = get_data_manager.get_results()
    if len(results) > 0:
        finish_num += len(results)
        for galaxy_data in results:
            dummy_results.append(galaxy_data.seed_id)
        print(f"当前进度{finish_num}/{task_num}", end="\r")
        sleep(0.01)
    else:
        sleep(0.1)
tag = perf_counter() - tag
print(f"用时{tag:.2f}s, 速度{task_num/tag:.2f}seed/s")
