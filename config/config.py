import json

from .cfg_dict_tying import GUIConfig



class Config:
    def __init__(self, cfg_file_path: str = "./config.json"):
        try:
            with open(cfg_file_path, "r", encoding="utf-8") as f:
                cfg_dict = json.load(f)
            self.config = GUIConfig(**cfg_dict)
        except FileNotFoundError:
            print("配置文件不存在，使用默认配置")
            self.config = GUIConfig()
        except Exception as e:
            self.config = GUIConfig()
            print(f"加载配置文件失败: {e}")
            raise e
    
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
