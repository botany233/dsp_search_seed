from CApi import search_seed
import json

galaxy_data = search_seed.get_galaxy_data(233, 32)
print(type(galaxy_data))

galaxy_json = json.dumps(galaxy_data, ensure_ascii=False, indent=4)
print(galaxy_json)

<<<<<<< Updated upstream
with open("example.txt", "w", encoding="utf-8") as f:
    f.write(galaxy_json)
=======
with open("example.json", "w", encoding="utf-8") as f:
    f.write(galaxy_json)

galaxy_data = json.loads(galaxy_json)
print(galaxy_data)
>>>>>>> Stashed changes
