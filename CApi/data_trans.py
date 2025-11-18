import inspect
from .search_seed import GalaxyData, StarData, PlanetData

def data_to_dict(galaxy_data: GalaxyData) -> dict:
    galaxy_dict = {"stars": [], "planets": []}
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
        elif galaxy_name == "planets":
            for planet_data in galaxy_value:
                planet_dict = {}
                for planet_name, planet_value in inspect.getmembers(planet_data):
                    if planet_name.startswith("_"):
                        continue
                    planet_dict[planet_name] = planet_value
                galaxy_dict["planets"].append(planet_dict)
        else:
            galaxy_dict[galaxy_name] = galaxy_value
    if not galaxy_dict["planets"]:
        galaxy_dict.pop("planets")
    return galaxy_dict

def dict_to_data(galaxy_dict: dict) -> GalaxyData:
    galaxy_data = GalaxyData()
    for galaxy_name, galaxy_value in galaxy_dict.items():
        if galaxy_name == "stars":
            stars = []
            for star_dict in galaxy_value:
                star_data = StarData()
                for star_name, star_value in star_dict.items():
                    if star_name == "planets":
                        planets = []
                        for planet_dict in star_value:
                            planet_data = PlanetData()
                            for planet_name, planet_value in planet_dict.items():
                                setattr(planet_data, planet_name, planet_value)
                            planets.append(planet_data)
                        setattr(star_data, "planets", planets)
                    else:
                        setattr(star_data, star_name, star_value)
                stars.append(star_data)
            setattr(galaxy_data, "stars", stars)
        else:
            setattr(galaxy_data, galaxy_name, galaxy_value)
    return galaxy_data
