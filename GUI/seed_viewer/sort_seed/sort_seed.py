from PySide6.QtCore import QThread, Signal
from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
from .sort_seed_util import get_value_function
from config import cfg
from time import sleep, perf_counter

from logger import log
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from GUI.seed_viewer.MainInterface import ViewerInterface

class SortThread(QThread):
    finished = Signal()
    label_text = Signal(str)
    completed = Signal()

    def __init__(self, parent: 'ViewerInterface'):
        super().__init__(parent)
        self.seed_list = parent.seed_list
        self.main_type_combo = parent.main_type_combo
        self.sub_type_combo = parent.sub_type_combo
        self.quick_sort_switch = parent.quick_sort_switch
        self.running = False
        self.end_flag = False

    def terminate(self) -> None:
        self.end_flag = True

    def isRunning(self) -> bool:
        return self.running

    def run(self):
        try:
            self.running = True
            flag = perf_counter()
            self.label_text.emit("正在生成任务...")

            finish_num, task_num = 0, len(self.seed_list)
            value_func = get_value_function(self.main_type_combo.currentText(), self.sub_type_combo.currentText())

            get_data_manager = GetDataManager(min(cpu_count(), cfg.config.max_thread), self.quick_sort_switch.isChecked(), 128)
            seed_index_dict = {}
            for index, (seed_id, star_num, _) in enumerate(self.seed_list):
                seed_index_dict[(seed_id, star_num)] = index
                get_data_manager.add_task(seed_id, star_num)

                if self.end_flag:
                    get_data_manager.shutdown()
                    break
            else:
                self.label_text.emit(f"0/{task_num}(0%)")
                while finish_num < task_num:
                    if self.end_flag:
                        get_data_manager.shutdown()
                        break

                    results = get_data_manager.get_results()
                    if len(results) > 0:
                        for galaxy_data in results:
                            value = value_func(galaxy_data)
                            index = seed_index_dict[(galaxy_data.seed, galaxy_data.star_num)]
                            self.seed_list[index][2] = value
                        finish_num += len(results)
                        self.label_text.emit(f"{finish_num}/{task_num}({round(finish_num/task_num*100)}%)")
                        sleep(0.01)
                    else:
                        sleep(0.1)
            print(f"排序完成，用时{perf_counter() - flag:.2f}s")

            if self.end_flag:
                self.label_text.emit("排序已取消")
            else:
                self.completed.emit()
        except Exception as e:
            log.error(f"Sort failed: {e}")
        finally:
            self.end_flag = False
            self.running = False
            self.finished.emit()
