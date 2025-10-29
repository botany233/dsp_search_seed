from PySide6.QtCore import QThread
from CApi import *
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count
from .sort_seed_util import get_seed_value, get_value_function

from logger import log

class SortThread(QThread):
    def __init__(self, parent):
        super().__init__(parent)
        self.process_label = parent.progress_label
        self.seed_list = parent.seed_list
        self.seed_list_lock = parent.seed_list_lock
        self.seed_scroll = parent.seed_scroll
        self.main_type_combo = parent.main_type_combo
        self.sub_type_combo = parent.sub_type_combo
        self.sort_order_switch = parent.sort_order_switch
        self.start_button = parent.start_button
        self.start_button.clicked.connect(self.__on_start_button_clicked)
        self.stop_button = parent.stop_button
        self.stop_button.clicked.connect(self.__on_stop_button_clicked)
        self.running = False
        self.end_flag = False

    def __on_start_button_clicked(self):
        if self.running:
            return
        self.start()

    def __on_stop_button_clicked(self):
        self.terminate()

    def terminate(self) -> None:
        self.end_flag = True

    def isRunning(self) -> bool:
        return self.running

    def run(self):
        try:
            self.seed_list_lock.acquire()
            self.running = True
            self.start_button.setEnabled(False)
            self.stop_button.setEnabled(True)

            seed_list_len = len(self.seed_list)
            self.process_label.setText("正在生成任务...")
            value_func = get_value_function(self.main_type_combo.currentText(), self.sub_type_combo.currentText())
            futures = []
            with ProcessPoolExecutor(max_workers = min(seed_list_len // 10, cpu_count())) as executor:
                for seed in self.seed_list:
                    futures.append(executor.submit(get_seed_value, seed[0], seed[1], value_func))

                    if self.end_flag:
                        executor.shutdown(wait=False, cancel_futures=True)
                        break

                self.process_label.setText(f"0/{seed_list_len}(0%)")
                for index, future in enumerate(futures):
                    self.seed_list[index][2] = future.result()
                    if index % max(1, seed_list_len // 1000) == 0:
                        self.process_label.setText(f"{index+1}/{seed_list_len}({round((index+1)/seed_list_len*100)}%)")

                    if self.end_flag:
                        executor.shutdown(wait=False, cancel_futures=True)
                        break

            self.process_label.setText(f"排序中...")
            self.seed_scroll.do_sort(self.sort_order_switch.isChecked())
            self.process_label.setText(f"排序完成！")
        except Exception as e:
            log.error(f"Search failed: {e}")
        finally:
            self.seed_list_lock.release()
            self.end_flag = False
            self.running = False
            self.stop_button.setEnabled(False)
            self.start_button.setEnabled(True)
