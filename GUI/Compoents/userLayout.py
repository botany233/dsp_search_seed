from collections import deque

from PySide6.QtWidgets import QVBoxLayout, QHBoxLayout, QFileDialog
from PySide6.QtGui import QColor
from PySide6.QtCore import QTimer, Qt
from qfluentwidgets import PushButton, BodyLabel, ProgressBar, CaptionLabel, LineEdit

from GUI.Messenger import SearchMessages
from config import cfg

from .Widgets.button import ConfigSwitchButton

class UserLayout(QVBoxLayout):
    def __init__(self):
        super().__init__()
        self.userButtonsLayout = QHBoxLayout()

        self.addLayout(self.userButtonsLayout)
        self.addStretch()

        self.search_mode_switch = ConfigSwitchButton("范围搜索", indicatorPos=1)
        self.search_mode_switch.setOnText("二次搜索")
        self.search_mode_switch.set_config(config_obj=cfg.config, config_key="search_mode")
        self.userButtonsLayout.addWidget(self.search_mode_switch)

        self.quick_check_switch = ConfigSwitchButton("标准模式", indicatorPos=1)
        self.quick_check_switch.setOnText("快速模式")
        self.quick_check_switch.set_config(config_obj=cfg.config, config_key="quick_check")
        self.userButtonsLayout.addWidget(self.quick_check_switch)

        self.outputFileLabel = BodyLabel("输出文件名称 ")
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
        self.seedInfoLabel.setText(f"累计找到种子数: {total_seed_num}  上次命中的种子: {last_seed_id}, {last_star_num}")

    def _update_progress(self, finish_task: int, total_task: int, use_time: float):
        if finish_task < total_task:
            self.progressBar.setValue(finish_task)

            progress_str = f"搜索进度: {finish_task}/{total_task}({finish_task * 100 // total_task}%)"
            remain_time_str = self.get_remain_time_str(finish_task, total_task, use_time)
            self.barLabel.setText(progress_str + "  " + remain_time_str)
        elif finish_task == total_task:
            self.progressBar.setValue(self.progressBar.maximum())
            self.barLabel.setText(f"搜索完成！用时{self.get_format_time_str(use_time)}")
        else:
            raise ValueError(f"finish_task={finish_task}不能大于total_task={total_task}！")

    def get_remain_time_str(self, batch_id: int, total_batch: int, use_time: float) -> str:
        if len(self.progress_cache) == 0 or batch_id > self.progress_cache[-1][0]:
            self.progress_cache.append((batch_id, use_time))
        while use_time - self.progress_cache[0][1] > 60:
            self.progress_cache.popleft()
        cost_time_str = self.get_format_time_str(use_time)
        if batch_id <= 0:
            leave_time_str = "?"
            speed_str = "?seed/s"
        else:
            last = self.progress_cache[0]
            speed = (batch_id - last[0]) / (use_time - last[1])
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
