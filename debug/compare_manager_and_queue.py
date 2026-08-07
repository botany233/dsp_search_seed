import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
import json
import time
import random

from tqdm import tqdm

from CApi import *

def main():
    test_num = 1000
    max_thread = 8

    data_manager = GetDataManager(8, False, 128)
    data_queue = GetDataQueue(128)
    for _ in tqdm(range(test_num)):
        seed_id = random.randint(0, 99999999)
        star_num = random.randint(32, 64)
        resource_index = random.randint(0, 10)
        seed = Seed(seed_id, star_num, resource_index)

        data_manager.add_task(seed)
        data_queue.add_task(seed, max_thread)

        while True:
            manager_result = data_manager.get_results()
            if manager_result:
                manager_result = json.dumps(data_to_dict(manager_result[0]), ensure_ascii=False, indent=4)
                break
            time.sleep(0.05)

        while True:
            queue_result = data_queue.get_results()
            if queue_result:
                queue_result = json.dumps(data_to_dict(queue_result[0]), ensure_ascii=False, indent=4)
                break
            time.sleep(0.05)

        if manager_result != queue_result:
            seed_info = f"{seed_id}_{star_num}_{resource_index}"
            with open(f"{seed_info}_data_manager.json", "w", encoding="utf-8") as f:
                f.write(manager_result)

            with open(f"{seed_info}_data_queue.json", "w", encoding="utf-8") as f:
                f.write(queue_result)

            raise Exception(f"Mismatch in seed {seed_id}-{star_num}-{resource_index}")

if __name__ == "__main__":
    main()
