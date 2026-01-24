from CApi import *
import random
from tqdm import tqdm

def check_veins(seed_id: int, star_num: int):
    set_device_id_c(-1)
    galaxy_data_cpu = search_seed.get_galaxy_data_c(seed_id, star_num, False)
    set_device_id_c(0)
    galaxy_data_gpu = search_seed.get_galaxy_data_c(seed_id, star_num, False)
    for star_data_cpu, star_data_gpu in zip(galaxy_data_cpu.stars, galaxy_data_gpu.stars):
        for planet_data_cpu, planet_data_gpu in zip(star_data_cpu.planets, star_data_gpu.planets):
            for i in range(14):
                if planet_data_cpu.veins_point[i] != planet_data_gpu.veins_point[i]:
                    print(f"Mismatch found on planet {planet_data_cpu.name} for vein index {i}: CPU={planet_data_cpu.veins_point[i]}, GPU={planet_data_gpu.veins_point[i]}")

if __name__ == "__main__":
    test_num = 1000
    for _ in tqdm(range(test_num)):
        seed_id = random.randint(0, 99999999)
        star_num = random.randint(32, 64)
        check_veins(seed_id, star_num)
