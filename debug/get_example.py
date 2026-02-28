import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
import json

from CApi import *

galaxy_data = search_seed.get_galaxy_data_c(191, 64, False)
galaxy_dict = data_to_dict(galaxy_data)
galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
with open(os.path.join(os.path.dirname(__file__), "example.json"), "w", encoding="utf-8") as f:
    f.write(galaxy_json)

# for star in galaxy_dict["stars"]:
#     for planet in star["planets"]:
#         if planet["veins_point"][11] >= 200:
#             print(planet["name"], "光栅")
#         if planet["veins_point"][12] >= 20 and planet["veins_point"][9] >= 20:
#             print(planet["name"], "紫糖")
