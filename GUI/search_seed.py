from PySide6.QtCore import QThread, QMutex
from .Messenger import SearchMessages
from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
from time import perf_counter
from math import ceil
from datetime import datetime
from collections import deque
import json

from config import cfg
from .config_to_condition import get_galaxy_condition
from logger import log

class SearchThread(QThread):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.mutex = QMutex()
        self.running = False
        self.end_flag = False

    def terminate(self) -> None:
        self.end_flag = True

    def isRunning(self) -> bool:
        return self.running

    def run(self):
        try:
            self.mutex.lock()
            self.running = True

            gui_cfg = cfg.copy()
            galaxy_condition = get_galaxy_condition(gui_cfg.galaxy_condition)
            seeds = (gui_cfg.start_seed, gui_cfg.end_seed)
            star_nums = (gui_cfg.start_star_num, gui_cfg.end_star_num)
            batch_size = gui_cfg.batch_size
            max_thread = gui_cfg.max_thread
            save_name = gui_cfg.save_name + ".csv"

            if not self.end_flag:
                self.search(galaxy_condition, seeds, star_nums, batch_size, max_thread, save_name)
        except Exception as e:
            log.error(f"Search failed: {e}")
        finally:
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
        galaxy_condition = change_galaxy_condition_legal(galaxy_condition)
        galaxy_condition_simple = get_galaxy_condition_simple(galaxy_condition)

        galaxy_str = json.dumps(galaxy_condition, ensure_ascii = False)
        galaxy_str_simple = json.dumps(galaxy_condition_simple, ensure_ascii = False)

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
                    executor.shutdown(cancel_futures=True)
                    break

                seed += batch_size
                if seed <= seeds[1]:
                    futures.append(executor.submit(check_batch_c, seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_str, galaxy_str_simple))
            else:
                SearchMessages.searchEndNormal.emit(perf_counter() - start_time)
