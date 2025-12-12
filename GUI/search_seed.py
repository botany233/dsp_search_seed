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
from .config_to_condition import config_to_galaxy_condition
from .Compoents.seed_manager import SeedManager
from logger import log

class SearchThread(QThread):
    def __init__(self, seed_manager: SeedManager, parent=None):
        super().__init__(parent)
        self.mutex = QMutex()
        self.running = False
        self.end_flag = False
        self.seed_manager = seed_manager

    def terminate(self) -> None:
        self.end_flag = True

    def isRunning(self) -> bool:
        return self.running

    def run(self):
        try:
            self.mutex.lock()
            self.running = True

            gui_cfg = cfg.copy()
            galaxy_condition = config_to_galaxy_condition(gui_cfg.galaxy_condition)
            save_name = gui_cfg.save_name + ".csv"
            batch_size = gui_cfg.batch_size
            max_thread = gui_cfg.max_thread
            quick = gui_cfg.quick_check
            device_id = gui_cfg.device_id
            local_size = gui_cfg.local_size

            if gui_cfg.search_mode == 0:
                seeds = (gui_cfg.start_seed, gui_cfg.end_seed)
                star_nums = (gui_cfg.start_star_num, gui_cfg.end_star_num)

                if not self.end_flag:
                    self.range_search(galaxy_condition, seeds, star_nums, batch_size, max_thread, save_name, quick, device_id, local_size)
            else:
                if not self.end_flag:
                    self.precise_search(galaxy_condition, batch_size, max_thread, save_name, quick, device_id, local_size)
        except Exception as e:
            log.error(f"Search failed: {e}")
        finally:
            self.mutex.unlock()
            self.end_flag = False
            self.running = False
            SearchMessages.searchEnd.emit()

    def precise_search(self,
                galaxy_condition: dict,
                batch_size: int,
                max_thread: int,
                save_name: str,
                quick: bool,
                device_id: int,
                local_size: int) -> None:
        galaxy_condition = change_galaxy_condition_legal(galaxy_condition)

        last_valid_seed, valid_seed_num = str(-1), 0
        total_batch = ceil(self.seed_manager.get_seeds_count() / batch_size)
        start_time = perf_counter()
        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        SearchMessages.search_progress_info.emit(0, total_batch, 0, last_valid_seed, start_time, perf_counter())
        real_thread = min(max_thread, cpu_count())
        generator = self.seed_manager.get_all_seeds(batch_size)
        with ProcessPoolExecutor(max_workers = real_thread, initializer=init_process, initargs=(device_id, local_size)) as executor:
            futures = deque()
            for _ in range(real_thread * 10):
                try:
                    seeds_list, star_num_list = next(generator)
                    futures.append(executor.submit(check_precise_c, seeds_list, star_num_list, galaxy_condition, quick))
                except Exception:
                    break

            index = 0
            while (len(futures) > 0):
                result = futures.popleft().result()
                index += 1

                with open(save_name, "a", encoding="utf-8") as f:
                    f.writelines(map(lambda x: f"{x}\n", result))

                if result:
                    last_valid_seed = result[-1]
                    valid_seed_num += len(result)
                SearchMessages.search_progress_info.emit(index, total_batch, valid_seed_num, last_valid_seed, start_time, perf_counter())

                if self.end_flag:
                    executor.shutdown(cancel_futures=True)
                    break

                try:
                    seeds_list, star_num_list = next(generator)
                    futures.append(executor.submit(check_precise_c, seeds_list, star_num_list, galaxy_condition, quick))
                except Exception:
                    continue
            else:
                SearchMessages.searchEndNormal.emit(perf_counter() - start_time)

    def range_search(self,
               galaxy_condition: dict,
               seeds: tuple[int, int],
               star_nums: tuple[int, int],
               batch_size: int,
               max_thread: int,
               save_name: str,
               quick: bool,
               device_id: int,
               local_size: int) -> None:
        galaxy_condition = change_galaxy_condition_legal(galaxy_condition)

        last_seed, total_seed_num, total_batch = str(-1), 0, ceil((seeds[1]-seeds[0]+1)/batch_size)
        start_time = perf_counter()
        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        SearchMessages.search_progress_info.emit(0, total_batch, total_seed_num, last_seed, start_time, perf_counter())
        real_thread = min(max_thread, cpu_count())
        with ProcessPoolExecutor(max_workers = real_thread, initializer=init_process, initargs=(device_id, local_size)) as executor:
            futures = deque()
            for seed in range(seeds[0], min(seeds[1]+1, seeds[0]+batch_size*real_thread*10+1), batch_size):
                futures.append(executor.submit(check_batch_c, seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_condition, quick))
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
                    futures.append(executor.submit(check_batch_c, seed, min(seed+batch_size, seeds[1]+1), star_nums[0], star_nums[1]+1, galaxy_condition, quick))
            else:
                SearchMessages.searchEndNormal.emit(perf_counter() - start_time)
