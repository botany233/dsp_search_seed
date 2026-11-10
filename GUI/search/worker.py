from PySide6.QtCore import QThread, QMutex
from ..Messenger import SearchMessages
from CApi import SeedManager, change_condition_legal, resource_rate_c, CheckPreciseManager, CheckBatchManager
from multiprocessing import cpu_count
from time import perf_counter, sleep
from datetime import datetime

from config.cfg_dict_tying import GUIConfig
from logger import log

from .config_to_condition import config_to_galaxy_condition, config_to_advanced_condition
from .time_record import TimeRecord

class SearchThread(QThread):
    def __init__(self, seed_manager: SeedManager, parent=None):
        super().__init__(parent)
        self.mutex = QMutex()
        self.end_flag = False
        self.wait_flag = False
        self.seed_manager = seed_manager
        self.task_config: GUIConfig | None = None

    def prepare_task(self, config: GUIConfig):
        """在 GUI 线程调用，继续暂停任务时不重新创建快照。"""
        if self.isRunning():
            raise RuntimeError("搜索运行期间不能替换任务快照")
        self.task_config = config.model_copy(deep=True)

    def terminate(self) -> None:
        self.end_flag = True
    
    def start_wait(self):
        self.wait_flag = True
    
    def end_wait(self):
        self.wait_flag = False

    def run(self):
        try:
            self.mutex.lock()

            if self.task_config is None:
                raise RuntimeError("搜索任务缺少启动快照")
            task_config = self.task_config
            gui_cfg = task_config.active_search
            shared_cfg = task_config.shared_search
            if task_config.condition_mode == "basic":
                galaxy_condition = config_to_galaxy_condition(gui_cfg.galaxy_condition)
            else:
                galaxy_condition = config_to_advanced_condition(gui_cfg.condition)
            galaxy_condition = change_condition_legal(galaxy_condition)
            save_name = shared_cfg.save_name + ".csv"
            max_thread = task_config.max_thread
            quick = shared_cfg.quick_check
            resource_index = resource_rate_c.index(shared_cfg.resource_rate)

            if shared_cfg.search_mode == 0:
                seeds = (shared_cfg.start_seed, shared_cfg.end_seed)
                star_nums = (shared_cfg.start_star_num, shared_cfg.end_star_num)

                if not self.end_flag:
                    self.range_search(galaxy_condition, seeds, star_nums, resource_index, max_thread, save_name, quick)
            else:
                if not self.end_flag:
                    self.precise_search(galaxy_condition, resource_index, max_thread, save_name, quick)
        except Exception as e:
            log.error(f"Search failed: {e}")
        finally:
            self.mutex.unlock()
            SearchMessages.searchEnd.emit()

    def precise_search(self,
                galaxy_condition: dict,
                resource_index: int,
                max_thread: int,
                save_name: str,
                quick: bool) -> None:
        time_record = TimeRecord()
        start_date_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        task_num = self.seed_manager.get_seeds_count()
        SearchMessages.new_find_seed.emit(0, -1, -1)
        SearchMessages.search_progress_info.emit(0, task_num, time_record.get_time())

        check_precise_manager = CheckPreciseManager(
            self.seed_manager, resource_index, galaxy_condition, quick, min(max_thread, cpu_count())
        )
        check_precise_manager.run()

        result_num = 0
        last_wait_flag = False
        while check_precise_manager.is_running():
            sleep(0.1)
            if self.end_flag:
                check_precise_manager.shutdown()
                break
            if last_wait_flag != self.wait_flag:
                last_wait_flag = self.wait_flag
                if self.wait_flag:
                    check_precise_manager.start_wait()
                    time_record.pause()
                else:
                    check_precise_manager.end_wait()
                    time_record.resume()

            if (new_result_num := check_precise_manager.get_result_num()) > result_num:
                result_num = new_result_num
                last_result = check_precise_manager.get_last_result()
                SearchMessages.new_find_seed.emit(result_num, last_result.seed_id, last_result.star_num)
            SearchMessages.search_progress_info.emit(check_precise_manager.get_task_progress(), task_num, time_record.get_time())
        else:
            result_num = check_precise_manager.get_result_num()
            last_result = check_precise_manager.get_last_result()
            SearchMessages.new_find_seed.emit(result_num, last_result.seed_id, last_result.star_num)
            SearchMessages.search_progress_info.emit(task_num, task_num, time_record.get_time())

        results = check_precise_manager.get_results()
        results = sorted(results, key=lambda x: x.seed_id * 33 + x.star_num)
        if results:
            last_result = results[-1]
            SearchMessages.new_find_seed.emit(len(results), last_result.seed_id, last_result.star_num)

        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {start_date_time}\n")
            f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", results))

    def range_search(self,
               galaxy_condition: dict,
               seeds: tuple[int, int],
               star_nums: tuple[int, int],
               resource_index: int,
               max_thread: int,
               save_name: str,
               quick: bool) -> None:
        time_record = TimeRecord()
        start_date_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        task_num = (seeds[1] - seeds[0] + 1) * (star_nums[1] - star_nums[0] + 1)
        SearchMessages.new_find_seed.emit(0, -1, -1)
        SearchMessages.search_progress_info.emit(0, task_num, time_record.get_time())

        check_batch_manager = CheckBatchManager(
            seeds[0], seeds[1]+1, star_nums[0], star_nums[1]+1, resource_index,
            galaxy_condition, quick, min(max_thread, cpu_count())
        )
        check_batch_manager.run()

        result_num = 0
        last_wait_flag = False
        while check_batch_manager.is_running():
            sleep(0.1)
            if self.end_flag:
                check_batch_manager.shutdown()
                break
            if last_wait_flag != self.wait_flag:
                last_wait_flag = self.wait_flag
                if self.wait_flag:
                    check_batch_manager.start_wait()
                    time_record.pause()
                else:
                    check_batch_manager.end_wait()
                    time_record.resume()

            if (new_result_num := check_batch_manager.get_result_num()) > result_num:
                result_num = new_result_num
                last_result = check_batch_manager.get_last_result()
                SearchMessages.new_find_seed.emit(result_num, last_result.seed_id, last_result.star_num)
            SearchMessages.search_progress_info.emit(check_batch_manager.get_task_progress(), task_num, time_record.get_time())
        else:
            result_num = check_batch_manager.get_result_num()
            last_result = check_batch_manager.get_last_result()
            SearchMessages.new_find_seed.emit(result_num, last_result.seed_id, last_result.star_num)
            SearchMessages.search_progress_info.emit(task_num, task_num, time_record.get_time())

        results = check_batch_manager.get_results()
        results = sorted(results, key=lambda x: x.seed_id * 33 + x.star_num)
        if results:
            last_result = results[-1]
            SearchMessages.new_find_seed.emit(len(results), last_result.seed_id, last_result.star_num)

        with open(save_name, "a", encoding="utf-8") as f:
            f.write(f"#search seed time {start_date_time}\n")
            f.writelines(map(lambda x: f"{x.seed_id}, {x.star_num}\n", results))
