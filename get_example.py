from CApi import search_seed
import json

galaxy_json = search_seed.single_search(233, 32)

with open("example.txt", "w", encoding="utf-8") as f:
    f.write(galaxy_json)

galaxy_data = json.loads(galaxy_json)
print(galaxy_data)