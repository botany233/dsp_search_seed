from PySide6.QtCore import QThread, Signal
from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
from .sort_seed_util import get_seed_value, get_value_function

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
            is_quick_sort = self.quick_sort_switch.isChecked()
            seed_list_len = len(self.seed_list)
            self.label_text.emit("正在生成任务...")
            value_func = get_value_function(self.main_type_combo.currentText(), self.sub_type_combo.currentText())
            futures = []
            with ProcessPoolExecutor(max_workers = cpu_count()) as executor:
                for seed in self.seed_list:
                    futures.append(executor.submit(get_seed_value, seed[0], seed[1], value_func, is_quick_sort))

                    if self.end_flag:
                        executor.shutdown(cancel_futures=True)
                        break

                self.label_text.emit(f"0/{seed_list_len}(0%)")
                for index, future in enumerate(futures):
                    self.seed_list[index][2] = future.result()
                    if index % max(1, seed_list_len // 1000) == 0:
                        self.label_text.emit(f"{index+1}/{seed_list_len}({round((index+1)/seed_list_len*100)}%)")

                    if self.end_flag:
                        executor.shutdown(cancel_futures=True)
                        break

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
