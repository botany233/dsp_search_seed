import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
import random

from CApi import *
from tqdm import tqdm

def check_veins(seed: Seed):
    set_device_id_c(-1)
    galaxy_data_cpu = get_galaxy_data_c(seed, False)
    set_device_id_c(0)
    galaxy_data_gpu = get_galaxy_data_c(seed, False)
    for star_data_cpu, star_data_gpu in zip(galaxy_data_cpu.stars, galaxy_data_gpu.stars):
        for planet_data_cpu, planet_data_gpu in zip(star_data_cpu.planets, star_data_gpu.planets):
            for i in range(14):
                if abs(planet_data_cpu.veins_point[i] - planet_data_gpu.veins_point[i]) > 0:
                    print(f" Seed({seed_id}, {star_num}) in {planet_data_cpu.name}({planet_data_cpu.type}) vein {i} mismatch: CPU={planet_data_cpu.veins_point[i]}, GPU={planet_data_gpu.veins_point[i]}")

if __name__ == "__main__":
    test_num = 1000
    for _ in tqdm(range(test_num)):
        seed_id = random.randint(0, 99999999)
        star_num = random.randint(32, 64)
        resource_index = random.randint(0, 10)
        seed = Seed(seed_id, star_num, resource_index)
        check_veins(seed)
