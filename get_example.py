from CApi import *
import inspect
import json

def get_galaxy_dict(galaxy_data):
    galaxy_dict = {"stars": []}
    for galaxy_name, galaxy_value in inspect.getmembers(galaxy_data):
        if galaxy_name.startswith("_"):
            continue
        if galaxy_name == "stars":
            for star_data in galaxy_value:
                star_dict = {"planets": []}
                for star_name, star_value in inspect.getmembers(star_data):
                    if star_name.startswith("_"):
                        continue
                    if star_name == "planets":
                        for planet_data in star_value:
                            planet_dict = {}
                            for planet_name, planet_value in inspect.getmembers(planet_data):
                                if planet_name.startswith("_"):
                                    continue
                                planet_dict[planet_name] = planet_value
                            star_dict["planets"].append(planet_dict)
                    else:
                        star_dict[star_name] = star_value
                galaxy_dict["stars"].append(star_dict)
        else:
            galaxy_dict[galaxy_name] = galaxy_value
    return galaxy_dict

if __name__ == "__main__":
    galaxy_data = search_seed.get_galaxy_data_c(90779, 64)

    galaxy_dict = get_galaxy_dict(galaxy_data)

    galaxy_json = json.dumps(galaxy_dict, ensure_ascii=False, indent=4)
    with open("example.json", "w", encoding="utf-8") as f:
        f.write(galaxy_json)
