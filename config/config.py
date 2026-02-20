import json
import os

from .cfg_dict_tying import GUIConfig, GalaxyCondition, PlanetCondition, StarCondition


class Config:
    def __init__(self, cfg_file_path: str = "./config.json"):
        if os.path.exists(cfg_file_path):
            with open(cfg_file_path, "r", encoding="utf-8") as f:
                cfg_dict = json.load(f)
            self.config = GUIConfig(**cfg_dict)
        else:
            self.__init__config()
            self.save()

        self.version = "PLACEHOLDER_VERSION"
        if self.version == "PLACEHOLDER_VERSION":
            self.version = "dev"

    def __init__config(self):
        self.config = GUIConfig()
        self.config.galaxy_condition.star_condition = [StarCondition()]
        self.config.galaxy_condition.star_condition[0].planet_condition = [PlanetCondition()]
        self.config.galaxy_condition.star_condition[0].planet_condition[0].moon_conditions = [PlanetCondition()]
        self.config.galaxy_condition.planet_condition = [PlanetCondition()]
        self.config.galaxy_condition.planet_condition[0].moon_conditions = [PlanetCondition()]

    def save(self) -> None:
        with open("./config.json", "w", encoding="utf-8") as f:
            f.write(self.config.model_dump_json(ensure_ascii=False, indent=4))

    def cover(self, new_config: GUIConfig) -> None:
        self.config = GUIConfig.model_validate(new_config)
        self.save()

    def copy(self) -> GUIConfig:
        return self.config.model_copy()


if __name__ == "__main__":
    cfg = Config()

    cfg.config.batch_size = 5000
