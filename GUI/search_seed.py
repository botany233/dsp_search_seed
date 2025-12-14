from PySide6.QtCore import QThread, QMutex
from .Messenger import SearchMessages
from CApi import *
from multiprocessing import cpu_count
from time import perf_counter, sleep
from datetime import datetime

from config import cfg
from .config_to_condition import config_to_galaxy_condition
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
            galaxy_condition = change_galaxy_condition_legal(galaxy_condition)
            save_name = gui_cfg.save_name + ".csv"
            max_thread = gui_cfg.max_thread
            quick = gui_cfg.quick_check
            device_id = gui_cfg.device_id
            local_size = gui_cfg.local_size

            if gui_cfg.search_mode == 0:
                seeds = (gui_cfg.start_seed, gui_cfg.end_seed)
                star_nums = (gui_cfg.start_star_num, gui_cfg.end_star_num)

                if not self.end_flag:
                    self.range_search(galaxy_condition, seeds, star_nums, max_thread, save_name, quick, device_id, local_size)
            else:
                if not self.end_flag:
                    self.precise_search(galaxy_condition, max_thread, save_name, quick, device_id, local_size)
        except Exception as e:
            log.error(f"Search failed: {e}")
        finally:
            self.mutex.unlock()
            self.end_flag = False
            self.running = False
            SearchMessages.searchEnd.emit()

    def precise_search(self,
                galaxy_condition: dict,
                max_thread: int,
                save_name: str,
                quick: bool,
                device_id: int,
                local_size: int) -> None:
        start_time = perf_counter()
        start_date_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        task_num = self.seed_manager.get_seeds_count()
        SearchMessages.search_progress_info.emit(0, task_num, 0, "-1, -1", start_time, perf_counter())

        init_process(device_id, local_size)
        check_precise_manager = CheckPreciseManager(
            self.seed_manager, galaxy_condition, quick, min(max_thread, cpu_count())
        )
        check_precise_manager.run()

        result_num = 0
        while check_precise_manager.is_running():
            new_result_num = check_precise_manager.get_result_num()
            if new_result_num > result_num:
                result_num = new_result_num
                last_result = check_precise_manager.get_last_result()
                SearchMessages.search_progress_info.emit(
                    check_precise_manager.get_task_progress(),
                    task_num,
                    result_num,
                    f"{last_result.seed_id}, {last_result.star_num}",
                    start_time,
                    perf_counter()
                )

            if self.end_flag:
                check_precise_manager.shutdown()
                break
            sleep(0.05)
        else:
            SearchMessages.searchEndNormal.emit(perf_counter() - start_time)
        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {start_date_time}\n")
            results = check_precise_manager.get_results()
            results = sorted(results, key=lambda x: x.seed_id * 33 + x.star_num)
            f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", results))

    def range_search(self,
               galaxy_condition: dict,
               seeds: tuple[int, int],
               star_nums: tuple[int, int],
               max_thread: int,
               save_name: str,
               quick: bool,
               device_id: int,
               local_size: int) -> None:
        start_time = perf_counter()
        start_date_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        task_num = (seeds[1] - seeds[0] + 1) * (star_nums[1] - star_nums[0] + 1)
        SearchMessages.search_progress_info.emit(0, task_num, 0, "-1, -1", start_time, perf_counter())

        init_process(device_id, local_size)
        check_batch_manager = CheckBatchManager(
            seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1,
            galaxy_condition, quick, min(max_thread, cpu_count())
        )
        check_batch_manager.run()
        result_num = 0
        while check_batch_manager.is_running():
            new_result_num = check_batch_manager.get_result_num()
            if new_result_num > result_num:
                result_num = new_result_num
                last_result = check_batch_manager.get_last_result()
                SearchMessages.search_progress_info.emit(
                    check_batch_manager.get_task_progress(),
                    task_num,
                    result_num,
                    f"{last_result.seed_id}, {last_result.star_num}",
                    start_time,
                    perf_counter()
                )

            if self.end_flag:
                check_batch_manager.shutdown()
                break
            sleep(0.05)
        else:
            SearchMessages.searchEndNormal.emit(perf_counter() - start_time)

        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {start_date_time}\n")
            results = check_batch_manager.get_results()
            results = sorted(results, key=lambda x: x.seed_id * 33 + x.star_num)
            f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", results))
