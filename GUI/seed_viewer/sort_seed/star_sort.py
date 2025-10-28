from CApi import GalaxyData

star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
              "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星"]

def handle_star_sort(subtype: str) -> function:
    n = star_types.index(subtype)
    def get_star_value(galaxy_data: GalaxyData) -> int:
        return galaxy_data.star_type_nums[n]
    return get_star_value