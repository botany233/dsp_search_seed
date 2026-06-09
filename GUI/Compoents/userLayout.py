from collections import deque

from PySide6.QtWidgets import QVBoxLayout, QHBoxLayout, QFileDialog
from PySide6.QtGui import QColor
from PySide6.QtCore import QTimer, Qt
from qfluentwidgets import PushButton, BodyLabel, ProgressBar, CaptionLabel, LineEdit

from GUI.Messenger import SearchMessages
from config import cfg
from language import tr

from .Widgets.button import ConfigSwitchButton

class UserLayout(QVBoxLayout):
    def __init__(self):
        super().__init__()
        self.userButtonsLayout = QHBoxLayout()

        self.addLayout(self.userButtonsLayout)
        self.addStretch()

        self.search_mode_switch = ConfigSwitchButton(tr("search.status.range_mode"), indicatorPos=1)
        self.search_mode_switch.setOnText(tr("search.status.precise_mode"))
        self.search_mode_switch.set_config(config_obj=cfg.config, config_key="search_mode")
        self.userButtonsLayout.addWidget(self.search_mode_switch)

        self.quick_check_switch = ConfigSwitchButton(tr("search.status.standard_mode"), indicatorPos=1)
        self.quick_check_switch.setOnText(tr("search.status.quick_mode"))
        self.quick_check_switch.set_config(config_obj=cfg.config, config_key="quick_check")
        self.userButtonsLayout.addWidget(self.quick_check_switch)

        self.outputFileLabel = BodyLabel(tr("search.status.output_file_name"))
        self.userButtonsLayout.addWidget(self.outputFileLabel)
        self.outputFileLine = LineEdit()
        self.outputFileLine.setPlaceholderText(cfg.config.save_name)
        self.outputFileLine.textEdited.connect(self._update_output_filename)
        self.userButtonsLayout.addWidget(self.outputFileLine)

        self.barVLayout = QVBoxLayout()
        self.barLabelLayout = QHBoxLayout()
        self.seedInfoLabel = CaptionLabel("")
        self.barLabelLayout.addWidget(self.seedInfoLabel)
        self.barLabel = CaptionLabel("")
        self.barLabelLayout.addWidget(self.barLabel)
        self.barLabelLayout.setAlignment(self.barLabel, Qt.AlignRight)
        self.barLabelLayout.setAlignment(self.seedInfoLabel, Qt.AlignCenter)


        self.barVLayout.addLayout(self.barLabelLayout)
        self.progressBar = ProgressBar()
        self.progressBar.setValue(0)
        self.progressBar.setFixedHeight(9)
        self.barVLayout.addWidget(self.progressBar)
        self.userButtonsLayout.addLayout(self.barVLayout)
        self.userButtonsLayout.setStretchFactor(self.barVLayout, 2)

        self.setContentsMargins(7, 0, 7, 7)

        SearchMessages.search_progress_info.connect(self._update_progress)
        SearchMessages.new_find_seed.connect(self._update_last_seed)

        self.progress_cache = deque(maxlen=1000)

    def _update_output_filename(self, filename: str):
        cfg.config.save_name = "seed" if filename == "" else filename

    def _update_last_seed(self, total_seed_num: int, last_seed_id: int, last_star_num: int):
        self.seedInfoLabel.setText(tr("search.status.found_summary").format(total_seed_num=total_seed_num, last_seed_id=last_seed_id, last_star_num=last_star_num))

    def _update_progress(self, finish_task: int, total_task: int, use_time: float):
        if finish_task < total_task:
            if total_task > 1000000000:
                self.progressBar.setValue(finish_task // 10)
            else:
                self.progressBar.setValue(finish_task)

            progress_str = tr("search.status.progress").format(finish_task=finish_task, total_task=total_task, percent=finish_task * 100 // total_task)
            remain_time_str = self.get_remain_time_str(finish_task, total_task, use_time)
            self.barLabel.setText(progress_str + "  " + remain_time_str)
        elif finish_task == total_task:
            self.progressBar.setValue(self.progressBar.maximum())
            self.barLabel.setText(tr("search.status.completed").format(time=self.get_format_time_str(use_time)))
        else:
            raise ValueError(f"finish_task={finish_task}不能大于total_task={total_task}！")

    def get_remain_time_str(self, batch_id: int, total_batch: int, use_time: float) -> str:
        if len(self.progress_cache) == 0 or batch_id > self.progress_cache[-1][0]:
            self.progress_cache.append((batch_id, use_time))
        while use_time - self.progress_cache[0][1] > 60 and len(self.progress_cache) > 1:
            self.progress_cache.popleft()
        cost_time_str = self.get_format_time_str(use_time)
        if batch_id <= 0:
            leave_time_str = "?"
            speed_str = "?seed/s"
            self.progress_cache.clear()
            self.progress_cache.append((batch_id, use_time))
        else:
            last_batch_id, last_use_time = self.progress_cache[0]
            speed = (batch_id - last_batch_id) / (use_time - last_use_time + 1e-8)
            if speed > 200000:
                print(last_batch_id, batch_id, last_use_time, use_time, speed)
                print(self.progress_cache)
                raise ValueError("计算得到的速度过快，可能是因为时间记录出现了问题！")
            leave_time_str = self.get_format_time_str((total_batch-batch_id)/speed)
            if speed >= 100:
                speed_str = f"{speed:.1f}seed/s"
            elif speed >= 1:
                speed_str = f"{speed:.2f}seed/s"
            else:
                speed_str = f"{1/speed:.2f}s/seed"

        return f"[{cost_time_str}<{leave_time_str}, {speed_str}]"

    def get_format_time_str(self, time_second:float):
        time_second = int(time_second)
        hours = time_second // 3600
        minutes = (time_second % 3600) // 60
        seconds = time_second % 60
        if hours > 0:
            return f"{hours}:{minutes:02d}:{seconds:02d}"
        else:
            return f"{minutes:02d}:{seconds:02d}"
