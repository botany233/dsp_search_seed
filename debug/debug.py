import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from copy import deepcopy
from time import perf_counter, sleep

from CApi import *
from debug_condition import debug_condition_functions

def main():
    cpu_thread = 20
    gpu_thread = 4
    device_id = 1
    local_size = 256

    do_python = 1

    c_save_path = os.path.join(os.path.dirname(__file__), "debug_results_c.csv")
    py_save_path = os.path.join(os.path.dirname(__file__), "debug_results_py.csv")
    if os.path.exists(c_save_path):
        os.remove(c_save_path)
    if os.path.exists(py_save_path):
        os.remove(py_save_path)
    set_device_id_c(device_id)
    set_local_size_c(local_size)
    set_gpu_max_worker_c(gpu_thread)

    galaxy_conditions = []
    for condition_func in debug_condition_functions:
        galaxy_condition, seeds, star_nums, task_type = condition_func()
        if task_type & 1:
            galaxy_conditions.append((f"{condition_func.__name__}_quick", deepcopy(galaxy_condition), seeds, star_nums, True))
        if task_type & 2:
            galaxy_conditions.append((f"{condition_func.__name__}_standard", galaxy_condition, seeds, star_nums, False))

    for name, galaxy_condition, seeds, star_nums, quick in galaxy_conditions:
        galaxy_condition = change_galaxy_condition_legal(galaxy_condition)

        if do_python:
            flag = perf_counter()
            py_results = check_seeds_py(seeds, star_nums, galaxy_condition, quick, cpu_thread, device_id, local_size)
            with open(py_save_path, "a") as f:
                f.write(f"{name}:\n")
                f.writelines(map(lambda x: f"{x[0]}, {x[1]}\n", py_results))
            print(f"{name} 完成: py用时{perf_counter() - flag:.2f}s，找到{len(py_results)}个种子")

        flag = perf_counter()
        check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, galaxy_condition, quick, cpu_thread)
        check_batch_manager.run()
        while check_batch_manager.is_running():
            sleep(0.1)
        c_results = check_batch_manager.get_results()
        c_results = sorted(c_results, key=lambda x: x.seed_id * 33 + x.star_num)
        with open(c_save_path, "a") as f:
            f.write(f"{name}:\n")
            f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", c_results))
        print(f"{name} 完成: cpp用时{perf_counter() - flag:.2f}s，找到{len(c_results)}个种子")

if __name__ == "__main__":
    main()
