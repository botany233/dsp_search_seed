from CApi import search_seed
import json

galaxy_data = search_seed.get_galaxy_data_c(231, 32)
print(type(galaxy_data))

galaxy_json = json.dumps(json.loads(galaxy_data), ensure_ascii=False, indent=4)
# print(galaxy_json)

with open("example.json", "w", encoding="utf-8") as f:
    f.write(galaxy_json)
