"""ConditionManager 新格式的规范化入口。

根、逻辑子节点和绑定端点使用 {"type": 名称, "value": 字典}：
矿物、恒星、星球、绑定、逻辑依次转换为 C++ 编号 1..5。
恒星的 planets 和行星的 moons 仍是未包装的行星条件列表。
与旧入口一样原位补齐字段；名称转为 C++ 使用的掩码、编号和矿物数组。
"""

from .search_seed import do_init_c, set_device_id_c, set_local_size_c
from .const_values import (
    dsp_level_c,
    liquid_types_c,
    planet_types_c,
    singularity_c,
    star_types_c,
    vein_names_c,
    condition_type_c
)

__all__ = ["change_condition_legal", "init_process"]


def init_process(device_id: int, local_size: int):
    do_init_c()
    if not set_device_id_c(device_id):
        print("Set device id failed! Roll back to cpu!")
    set_local_size_c(local_size)


def change_condition_legal(condition: dict) -> dict:
    condition["type"] = condition_type_c.index(condition["type"]) + 1
    match condition["type"]:
        case 1: condition["value"] = change_mineral_condition_legal(condition["value"])
        case 2: condition["value"] = change_star_condition_legal(condition["value"])
        case 3: condition["value"] = change_planet_condition_legal(condition["value"])
        case 4: condition["value"] = change_bond_condition_legal(condition["value"])
        case 5: condition["value"] = change_logic_condition_legal(condition["value"])
        case _: raise ValueError(f"不支持的条件类型: {condition["type"]}")
    return condition


def change_veins_legal(veins: dict | list[int]) -> list[int]:
    if isinstance(veins, dict):
        return [veins.get(name, 0) for name in vein_names_c]
    if len(veins) != len(vein_names_c):
        raise ValueError("矿物数组必须包含 14 项")
    return veins


def change_mineral_condition_legal(condition: dict) -> dict:
    condition["veins_point"] = change_veins_legal(condition.get("veins_point", {}))
    condition["veins_amount"] = change_veins_legal(condition.get("veins_amount", {}))
    return condition


def change_star_condition_legal(condition: dict) -> dict:
    change_mineral_condition_legal(condition)
    condition["satisfy_num"] = condition.get("satisfy_num", 1)
    condition["distance"] = condition.get("distance", 1000)
    condition["dyson_lumino"] = condition.get("dyson_lumino", 0)
    condition["type"] = get_star_type_mask(condition["type"]) if "type" in condition else 0xFFFFFFFF
    condition["planets"] = [change_planet_condition_legal(child) for child in condition.get("planets", [])]
    return condition


def change_planet_condition_legal(condition: dict) -> dict:
    change_mineral_condition_legal(condition)
    condition["satisfy_num"] = condition.get("satisfy_num", 1)
    condition["type"] = get_planet_type_mask(condition["type"]) if "type" in condition else 0xFFFFFFFF
    condition["dsp_level"] = dsp_level_c.index(condition["dsp_level"]) + 1 if "dsp_level" in condition else 0
    condition["liquid"] = liquid_types_c.index(condition["liquid"]) + 1 if "liquid" in condition else 0
    condition["singularity"] = get_singularity_mask(condition["singularity"]) if "singularity" in condition else 0
    condition["moons"] = [change_planet_condition_legal(child) for child in condition.get("moons", [])]
    return condition


def change_bond_condition_legal(condition: dict) -> dict:
    condition["satisfy_num"] = condition.get("satisfy_num", 1)
    condition["distance"] = condition.get("distance", 1000)
    for key in ("con1", "con2"):
        if condition[key]["type"] not in ("恒星", "星球"):
            raise ValueError("绑定端点必须为恒星或行星条件")
        condition[key] = change_condition_legal(condition[key])
    return condition


def change_logic_condition_legal(condition: dict) -> dict:
    """默认 OR；satisfy_num=子项数为 AND，0 为空节点恒真或单子项 NOT。"""
    condition["satisfy_num"] = condition.get("satisfy_num", 1)
    children = condition["children"]
    if not children and condition["satisfy_num"] != 0:
        raise ValueError("空逻辑节点的 satisfy_num 必须为 0")
    if condition["satisfy_num"] == 0 and len(children) > 1:
        raise ValueError("satisfy_num 为 0 时逻辑节点最多有一个子条件")
    condition["children"] = [change_condition_legal(child) for child in children]
    return condition


def get_star_type_mask(star_type: str | list[str]) -> int:
    if isinstance(star_type, str):
        star_type = [star_type]
    result = 0
    for st in star_type:
        result |= 1 << star_types_c.index(st)
    return result


def get_planet_type_mask(planet_type: str | list[str]) -> int:
    if isinstance(planet_type, str):
        planet_type = [planet_type]
    result = 0
    for pt in planet_type:
        result |= 1 << planet_types_c.index(pt)
    return result


def get_singularity_mask(singularity: str | list[str]) -> int:
    if isinstance(singularity, str):
        singularity = [singularity]
    result = 0
    for s in singularity:
        result |= 1 << singularity_c.index(s)
    return result
