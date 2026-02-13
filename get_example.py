from CApi import *
import json

if __name__ == "__main__":
    galaxy_data = search_seed.get_galaxy_data_c(6514, 64, False)
    galaxy_dict = data_to_dict(galaxy_data)
    galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
    with open("example.json", "w", encoding="utf-8") as f:
        f.write(galaxy_json)

    # for star in galaxy_dict["stars"]:
    #     for planet in star["planets"]:
    #         if planet["veins_point"][11] >= 200:
    #             print(planet["name"], "光栅")
    #         if planet["veins_point"][12] >= 20 and planet["veins_point"][9] >= 20:
    #             print(planet["name"], "紫糖")
