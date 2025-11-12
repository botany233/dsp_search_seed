from PySide6.QtWidgets import QVBoxLayout, QHBoxLayout, QFileDialog
from PySide6.QtGui import QColor
from PySide6.QtCore import QTimer, Qt

from qfluentwidgets import PushButton, BodyLabel, ProgressBar, CaptionLabel, LineEdit

from config import cfg
from .Widgets.button import ConfigSwitchButton

from GUI.Messenger import SearchMessages

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
        SearchMessages.searchEndNormal.connect(self._on_search_finish)

    def _update_output_filename(self, filename: str):
        cfg.config.save_name = "seed" if filename == "" else filename

    def _on_search_finish(self, use_time: float):
        self.barLabel.setText(f"搜索完成！用时{self.get_format_time_str(use_time)}")
        self.progressBar.setValue(self.progressBar.maximum())

    def _update_progress(self, batch_id: int, total_batch: int, total_seed_num: int, last_seed: str, start_time: float, current_time: float):
        self.seedInfoLabel.setText(f"累计找到种子数: {total_seed_num}  上次命中的种子: {last_seed}")
        self.progressBar.setValue(batch_id)

        progress_str = f"搜索进度: {batch_id}/{total_batch}({batch_id * 100 // self.progressBar.maximum()}%)"
        remain_time_str = self.get_remain_time_str(batch_id, total_batch, start_time, current_time)
        self.barLabel.setText(progress_str + "  " + remain_time_str)

    def get_remain_time_str(self, batch_id: int, total_batch: int, start_time: float, current_time: float) -> str:
        cost_time_str = self.get_format_time_str(current_time - start_time)
        if batch_id <= 0:
            leave_time_str = "?"
            speed_str = "?batch/s"
        else:
            leave_time_str = self.get_format_time_str((current_time-start_time)/batch_id*(total_batch-batch_id))
            speed = batch_id / (current_time - start_time)
            if speed >= 1:
                speed_str = f"{speed:.2f}batch/s"
            else:
                speed_str = f"{1/speed:.2f}s/batch"

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