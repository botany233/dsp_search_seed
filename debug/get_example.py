import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
import json

from CApi import *

# galaxy_data = search_seed.get_galaxy_data_c(Seed(5092, 64, 10), True)
# galaxy_dict = data_to_dict(galaxy_data)
# galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
# with open("example.json", "w", encoding="utf-8") as f:
#     f.write(galaxy_json)

get_data_manager = GetDataManager(10, True, 1024)
get_data_manager.add_task(5092, 64, 10)
while not (result := get_data_manager.get_results()):
    pass

galaxy_data = result[0]
for star in galaxy_data.stars:
    print(star.type, len(star.planets))
