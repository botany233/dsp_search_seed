from PySide6.QtCore import QThread, Signal
from multiprocessing import cpu_count
from time import sleep, perf_counter
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
        self.resource_rate_combo = parent.resource_rate_combo
        self.end_flag = False

    def terminate(self) -> None:
        self.end_flag = True

    def run(self):
        try:
            from language import tr

            self.label_text.emit(tr("viewer.progress.generating_tasks"))

            resource_index = self.resource_rate_combo.current_resource_index()
            main_type = self.main_type_combo.current_value()
            sub_type = self.sub_type_combo.current_value()
            value_func = get_value_function(main_type, sub_type)

            fast = main_type in ("矿脉储量", "矿脉数量", "行星类别", "恒星类别")
            vein_mask = 1 << vein_names_c.index(sub_type) if main_type in ("矿脉储量", "矿脉数量") else 0
            gen_planet_num = 0 if main_type == "恒星类别" else -1
            max_thread = min(cpu_count()-1, cfg.config.max_thread)
            quick = self.quick_sort_switch.isChecked()
            if fast:
                get_data_manager = GetDataManagerFast(
                    max_thread, quick, 1024, gen_planet_num=gen_planet_num, vein_mask=vein_mask,
                )
            else:
                get_data_manager = GetDataManager(max_thread, quick, 128)

            data = self.seed_list.get_all_data()
            task_num = len(data)
            time_tag = perf_counter()
            for seed_id, star_num, _ in data:
                get_data_manager.add_task(Seed(seed_id, star_num, resource_index))

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
                            self.seed_list.set_seed_value(galaxy_data.seed_id, galaxy_data.star_num, value)
                        finish_num += len(results)
                        self.label_text.emit(f"{finish_num}/{task_num}({finish_num/task_num:.0%})")
                        sleep(0.01)
                    else:
                        sleep(0.1)
            time_tag = perf_counter() - time_tag
            print(f"Sort completed in {time_tag:.2f} seconds, speed: {task_num/time_tag:.2f} seeds/s")

            if self.end_flag:
                self.label_text.emit(tr("viewer.progress.cancelled"))
            else:
                self.completed.emit()
        except Exception as e:
            log.error(f"Sort failed: {e}")
        finally:
            self.finished.emit()
