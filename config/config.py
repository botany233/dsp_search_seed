import json

from .cfg_dict_tying import GUIConfig, GalaxyCondition, PlanetCondition, StarSystemCondition


class Config:
    def __init__(self, cfg_file_path: str = "./config.json"):
        try:
            with open(cfg_file_path, "r", encoding="utf-8") as f:
                cfg_dict = json.load(f)
            self.config = GUIConfig(**cfg_dict)
        except FileNotFoundError:
            print("配置文件不存在，使用默认配置")
            self.__init__config()
            self.save()
        except json.JSONDecodeError as e:
            self.__init__config()
            print(f"配置文件格式错误，使用默认配置: {e}")
            self.save()
        except Exception as e:
            self.__init__config()
            print(f"加载配置文件失败: {e}")
            raise e

    def __init__config(self):
        self.config = GUIConfig()
        self.config.conditions.star_system_conditions = [StarSystemCondition()]
        self.config.conditions.planet_conditions = [PlanetCondition()]
        self.config.conditions.star_system_conditions[0].planet_conditions = [PlanetCondition()]

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
