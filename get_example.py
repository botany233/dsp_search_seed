from CApi import *
import inspect
import json

if __name__ == "__main__":
    galaxy_data = search_seed.get_galaxy_data_c(157, 64, False)
    galaxy_dict = data_to_dict(galaxy_data)
    galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
    with open("example.json", "w", encoding="utf-8") as f:
        f.write(galaxy_json)

    # for star in galaxy_dict["stars"]:
    #     for planet in star["planets"]:
    #         if planet["veins_point"][8] >= 20 and planet["veins_point"][12] >= 72 and planet["veins_point"][5] >= 78:
    #             print(planet["name"], "增产剂")
    #         if planet["veins_point"][9] >= 9 and planet["veins_point"][6] >= 14 and planet["veins_point"][12] >= 52 and planet["veins_point"][5] >= 14:
    #             print(planet["name"], "紫糖")
    #         if planet["veins_point"][11] >= 55 and planet["singularity"] & 1 == 1:
    #             print(planet["name"], "卡晶")
