import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from time import perf_counter, sleep

from CApi import *
from benchmark_condition import benchmark_condition_functions

# vein_names = "铁, 铜, 硅, 钛, 石, 煤, 油, 可燃冰, 金伯利, 分型硅, 有机晶体, 光栅石, 刺笋结晶, 单极磁石"
# liquid = "水, 硫酸"
# planet_types = ["地中海", "气态巨星", "冰巨星", "高产气巨", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩",
#                 "冰原冻土", "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩",
#                 "樱林海", "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]
# star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
#               "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星"]
# singularity = ["卫星", "多卫星", "潮汐锁定永昼永夜", "潮汐锁定1:2", "潮汐锁定1:4", "横躺自转", "反向自转"]

cpu_thread = 20
gpu_thread = 4
device_id = -1
local_size = 256

set_device_id_c(device_id)
set_local_size_c(local_size)
set_gpu_max_worker_c(gpu_thread)

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
    galaxy_condition = change_galaxy_condition_legal(galaxy_condition)
    flag = perf_counter()
    check_batch_manager = CheckBatchManager(seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, galaxy_condition, quick, cpu_thread)
    check_batch_manager.run()
    while check_batch_manager.is_running():
        sleep(0.1)
    result = check_batch_manager.get_results()
    result = sorted(result, key=lambda x: x.seed_id * 33 + x.star_num)
    flag = perf_counter() - flag
    print(f"{name} 完成，用时{flag:.2f}s，找到{len(result)}个种子，速度{(seeds[1]-seeds[0]+1)*(star_nums[1]-star_nums[0]+1) / flag:.3f}seed/s")
