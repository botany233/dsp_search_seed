from CApi import GalaxyDataSimple, star_types_c

def handle_star_sort(subtype: str):
    return GetStarValue(subtype)

class GetStarValue:
    def __init__(self, n):
        # 兼容旧的大类名称；细分类型直接读取对应的原生计数。
        giant_types = {
            "红巨星": ("M型红巨星", "K型红巨星"),
            "黄巨星": ("G型黄巨星", "F型黄巨星"),
            "蓝巨星": ("B型蓝巨星", "O型蓝巨星"),
            "白巨星": ("A型白巨星",),
        }
        self.indices = tuple(star_types_c.index(name) for name in giant_types.get(n, (n,)))

    def __call__(self, galaxy_data: GalaxyDataSimple) -> int:
        return sum(galaxy_data.star_type_num[index] for index in self.indices)
