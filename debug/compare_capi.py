import json
import random
import subprocess
import sys
from pathlib import Path

from tqdm import tqdm


PROJECT_ROOT = Path(__file__).resolve().parent.parent
WORKER_CODE = """
import importlib
import json
import sys

project_root, api_name, seed_id, star_num, resource_index, quick = sys.argv[1:]
sys.path.insert(0, project_root)

api = importlib.import_module(api_name)
api.set_device_id_c(-1)
seed = api.Seed(int(seed_id), int(star_num), int(resource_index))
galaxy_data = api.search_seed.get_galaxy_data_c(seed, quick == "1")
galaxy_dict = api.data_to_dict(galaxy_data)
print(json.dumps(galaxy_dict, ensure_ascii=True, sort_keys=True, separators=(",", ":")))
"""

# 这是使用示例
# galaxy_data = search_seed.get_galaxy_data_c(Seed(seed_id, star_num, resource_index), quick)
# galaxy_dict = data_to_dict(galaxy_data)
# galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
# with open("example.json", "w", encoding="utf-8") as f:
#     f.write(galaxy_json)


def _get_galaxy_json(
    api_name: str,
    seed_id: int,
    star_num: int,
    resource_index: int,
    quick: bool,
) -> str:
    try:
        result = subprocess.run(
            [
                sys.executable,
                "-c",
                WORKER_CODE,
                str(PROJECT_ROOT),
                api_name,
                str(seed_id),
                str(star_num),
                str(resource_index),
                "1" if quick else "0",
            ],
            cwd=PROJECT_ROOT,
            capture_output=True,
            check=True,
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.CalledProcessError as error:
        detail = error.stderr.strip() or error.stdout.strip() or "no error output"
        raise RuntimeError(f"{api_name} failed to generate galaxy data: {detail}") from error

    galaxy_json = result.stdout.strip()
    try:
        json.loads(galaxy_json)
    except json.JSONDecodeError as error:
        raise RuntimeError(f"{api_name} returned invalid JSON: {galaxy_json!r}") from error
    return galaxy_json


def compare(seed_id: int, star_num: int, resource_index: int, quick: bool) -> None:
    # 对比CApi和CApi_old的get_galaxy_data_c生成的结果是否完全一致，注意这两个pyd扩展命名空间有冲突，不能同时导入，可以考虑用子进程生成信息后变成json字符串再比较。如果不一样，则停止后续比较，同时保存为两个json供人工核验
    # 先用纯cpu跑
    capi_json = _get_galaxy_json("CApi", seed_id, star_num, resource_index, quick)
    old_capi_json = _get_galaxy_json("CApi_old", seed_id, star_num, resource_index, quick)
    if capi_json == old_capi_json:
        return

    case_name = f"seed_{seed_id}_{star_num}_{resource_index}_{quick}"
    output_paths = []
    for api_name, galaxy_json in (("CApi", capi_json), ("CApi_old", old_capi_json)):
        output_path = Path(__file__).resolve().parent / f"{case_name}_{api_name}.json"
        formatted_json = json.dumps(json.loads(galaxy_json), ensure_ascii=False, indent=4)
        output_path.write_text(f"{formatted_json}\n", encoding="utf-8")
        output_paths.append(output_path)

    raise AssertionError(
        f"CApi mismatch for Seed({seed_id}, {star_num}, {resource_index}), "
        f"quick={quick}; results saved to {output_paths[0]} and {output_paths[1]}"
    )


if __name__ == "__main__":
    test_num = 1000  # 测试脚本时候先跑10个就好
    for _ in tqdm(range(test_num)):
        seed_id = random.randint(0, 99999999)
        star_num = random.randint(32, 64)
        resource_index = random.randint(0, 10)
        quick = random.choice([True, False])
        compare(seed_id, star_num, resource_index, quick)
