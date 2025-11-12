from CApi import *
import inspect
import json

if __name__ == "__main__":
    galaxy_data = search_seed.get_galaxy_data_c(90779, 64, False)
    galaxy_dict = data_to_dict(galaxy_data)
    galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
    with open("example.json", "w", encoding="utf-8") as f:
        f.write(galaxy_json)
