from PySide6.QtCore import QThread, QMutex
from .Messenger import SearchMessages
from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
from time import perf_counter
from math import ceil
from datetime import datetime
from collections import deque

from config import cfg
from config.cfg_dict_tying import (
    GalaxyCondition,
    PlanetCondition,
    StarCondition,
    VeinsCondition,
    VeinsName,
    GUIConfig
)
from logger import log

class SearchThread(QThread):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.mutex = QMutex()
        self.running = False
        self.end_flag = False

    def terminate(self) -> None:
        self.mutex.unlock()
        self.end_flag = True

    def isRunning(self) -> bool:
        return self.running

    def run(self):
        # try:
            self.mutex.lock()
            self.running = True

            gui_cfg = cfg.copy()
            galaxy_condition = self.get_galaxy_condition(gui_cfg.galaxy_condition)
            seeds = (gui_cfg.start_seed, gui_cfg.end_seed)
            star_nums = (gui_cfg.start_star_num, gui_cfg.end_star_num)
            batch_size = gui_cfg.batch_size
            max_thread = gui_cfg.max_thread
            save_name = gui_cfg.save_name + ".csv"

            if not self.end_flag:
                self.search(galaxy_condition, seeds, star_nums, batch_size, max_thread, save_name)
        # except Exception as e:
        #     log.error(f"Search failed: {e}")
        # finally:
            self.mutex.unlock()
            self.end_flag = False
            self.running = False
            SearchMessages.searchEnd.emit()

    def search(self,
               galaxy_condition: dict,
               seeds: tuple[int, int],
               star_nums: tuple[int, int],
               batch_size: int,
               max_thread: int,
               save_name: str) -> None:
        galaxy_condition = change_condition_to_legal(galaxy_condition)
        galaxy_condition_simple = get_galaxy_condition_simple(galaxy_condition)

        galaxy_str = json.dumps(galaxy_condition, ensure_ascii = False)
        galaxy_str_simple = json.dumps(galaxy_condition_simple, ensure_ascii = False)
        print(galaxy_str)

        last_seed, total_seed_num, total_batch = str(-1), 0, ceil((seeds[1]-seeds[0]+1)/batch_size)
        start_time = perf_counter()
        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        SearchMessages.search_progress_info.emit(0, total_batch, total_seed_num, last_seed, start_time, perf_counter())
        real_thread = min(max_thread, cpu_count())
        with ProcessPoolExecutor(max_workers = real_thread) as executor:
            futures = deque()
            for seed in range(seeds[0], min(seeds[1]+1, seeds[0]+batch_size*real_thread*10+1), batch_size):
                futures.append(executor.submit(check_batch_c, seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_str, galaxy_str_simple))
            index = 0
            while (len(futures) > 0):
                result = futures.popleft().result()
                index += 1

                with open(save_name, "a", encoding="utf-8") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

                if result:
                    last_seed = result[-1]
                    total_seed_num += len(result)
                SearchMessages.search_progress_info.emit(index, total_batch, total_seed_num, last_seed, start_time, perf_counter())

                if self.end_flag:
                    executor.shutdown(wait=False, cancel_futures=True)
                    break

                seed += batch_size
                if seed <= seeds[1]:
                    futures.append(executor.submit(check_batch_c, seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_str, galaxy_str_simple))
            else:
                SearchMessages.searchEndNormal.emit(perf_counter() - start_time)

    def get_galaxy_condition(self, galaxy_cfg: GalaxyCondition) -> dict:
        galaxy_condition = {"stars": [], "planets": []}

        if galaxy_cfg.checked and (galaxy_veins := self.get_veins_dict(galaxy_cfg.veins_condition)):
            galaxy_condition["veins"] = galaxy_veins
        for star_cfg in galaxy_cfg.star_condition:
            star_condition = {"planets": []}
            if star_cfg.checked:
                if (star_veins := self.get_veins_dict(star_cfg.veins_condition)):
                    star_condition["veins"] = star_veins
                if star_cfg.star_type != "无限制":
                    star_condition["type"] = star_cfg.star_type
                if star_cfg.distance_level >= 0:
                    star_condition["distance"] = star_cfg.distance_level
                if star_cfg.lumino_level > 0:
                    star_condition["lumino"] = star_cfg.lumino_level
                if star_cfg.satisfy_num > 1:
                    star_condition["satisfy_num"] = star_cfg.satisfy_num
            for planet_cfg in star_cfg.planet_condition:
                planet_condition = {}
                if planet_cfg.checked:
                    if (planet_veins := self.get_veins_dict(planet_cfg.veins_condition)):
                        planet_condition["veins"] = planet_veins
                    if planet_cfg.planet_type != "无限制":
                        planet_condition["type"] = planet_cfg.planet_type
                    if planet_cfg.singularity != "无限制":
                        if planet_cfg.singularity == "永昼永夜":
                            planet_condition["singularity"] = "潮汐锁定永昼永夜"
                        else:
                            planet_condition["singularity"] = planet_cfg.singularity
                    if planet_cfg.liquid_type != "无限制":
                        planet_condition["liquid"] = planet_cfg.liquid_type
                    if planet_cfg.is_in_dsp:
                        planet_condition["is_in_dsp"] = True
                    if planet_cfg.is_on_dsp:
                        planet_condition["is_on_dsp"] = True
                    if planet_cfg.satisfy_num > 1:
                        planet_condition["satisfy_num"] = planet_cfg.satisfy_num
                star_condition["planets"].append(planet_condition)
            galaxy_condition["stars"].append(star_condition)

        for planet_cfg in galaxy_cfg.planet_condition:
            planet_condition = {}
            if planet_cfg.checked:
                if (planet_veins := self.get_veins_dict(planet_cfg.veins_condition)):
                    planet_condition["veins"] = planet_veins
                if planet_cfg.planet_type != "无限制":
                    planet_condition["type"] = planet_cfg.planet_type
                if planet_cfg.singularity != "无限制":
                    if planet_cfg.singularity == "永昼永夜":
                        planet_condition["singularity"] = "潮汐锁定永昼永夜"
                    else:
                        planet_condition["singularity"] = planet_cfg.singularity
                if planet_cfg.liquid_type != "无限制":
                    planet_condition["liquid"] = planet_cfg.liquid_type
                if planet_cfg.is_in_dsp:
                    planet_condition["is_in_dsp"] = True
                if planet_cfg.is_on_dsp:
                    planet_condition["is_on_dsp"] = True
                if planet_cfg.satisfy_num > 1:
                    planet_condition["satisfy_num"] = planet_cfg.satisfy_num
            galaxy_condition["planets"].append(planet_condition)

        return galaxy_condition

    @staticmethod
    def get_veins_dict(data: VeinsCondition) -> dict:
        veins = {}
        vein_names = VeinsName().model_dump()
        vein_data = data.model_dump()
        for key in vein_names.keys():
            if vein_data[key] > 0:
                veins[vein_names[key]] = vein_data[key]
        return veins