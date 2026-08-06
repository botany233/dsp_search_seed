import json
import random

from tqdm import tqdm

# 这是使用示例
# galaxy_data = search_seed.get_galaxy_data_c(Seed(seed_id, star_num, resource_index), quick)
# galaxy_dict = data_to_dict(galaxy_data)
# galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
# with open("example.json", "w", encoding="utf-8") as f:
#     f.write(galaxy_json)

def compare(seed_id: int, star_num: int, resource_index: int, quick: bool):
    # 对比CApi和CApi_old的get_galaxy_data_c生成的结果是否完全一致，注意这两个pyd扩展命名空间有冲突，不能同时导入，可以考虑用子进程生成信息后变成json字符串再比较。如果不一样，则停止后续比较，同时保存为两个json供人工核验
    pass

if __name__ == "__main__":
    test_num = 1000
    for _ in tqdm(range(test_num)):
        seed_id = random.randint(0, 99999999)
        star_num = random.randint(32, 64)
        resource_index = random.randint(0, 10)
        quick = random.choice([True, False])
        compare(seed_id, star_num, resource_index, quick)
