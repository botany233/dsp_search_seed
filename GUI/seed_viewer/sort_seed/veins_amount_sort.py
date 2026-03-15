from CApi import GalaxyData, vein_names_c

def handle_veins_amount_sort(subtype: str):
    n = vein_names_c.index(subtype)
    if subtype == "油":
        scale = 25000
    else:
        scale = 1
    return GetVeinsAmountValue(n, scale)

class GetVeinsAmountValue():
    def __init__(self, n, scale=1):
        self.n = n
        self.scale = scale

    def __call__(self, galaxy_data: GalaxyData) -> int:
        return galaxy_data.veins_amount[self.n] / self.scale
