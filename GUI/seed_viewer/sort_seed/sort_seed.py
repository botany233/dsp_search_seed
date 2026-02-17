from PySide6.QtCore import QThread, Signal
from multiprocessing import cpu_count
from time import sleep
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from GUI.seed_viewer.MainInterface import ViewerInterface

from config import cfg
from logger import log
from CApi import *
from .sort_seed_util import get_value_function

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
            self.label_text.emit("正在生成任务...")

            value_func = get_value_function(self.main_type_combo.currentText(), self.sub_type_combo.currentText())

            get_data_manager = GetDataManager(min(cpu_count(), cfg.config.max_thread), self.quick_sort_switch.isChecked(), 128)

            data = self.seed_list.get_all_data()
            for seed_id, star_num, _ in data:
                get_data_manager.add_task(seed_id, star_num)

                if self.end_flag:
                    get_data_manager.shutdown()
                    break
            else:
                finish_num, task_num = 0, len(data)
                self.label_text.emit(f"0/{task_num}(0%)")
                while finish_num < task_num:
                    if self.end_flag:
                        get_data_manager.shutdown()
                        break

                    results = get_data_manager.get_results()
                    if len(results) > 0:
                        for galaxy_data in results:
                            value = value_func(galaxy_data)
                            self.seed_list.set_seed_value(galaxy_data.seed, galaxy_data.star_num, value)
                        finish_num += len(results)
                        self.label_text.emit(f"{finish_num}/{task_num}({finish_num/task_num:.0%})")
                        sleep(0.01)
                    else:
                        sleep(0.1)

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
