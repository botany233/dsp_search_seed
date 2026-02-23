from time import sleep
from typing import Literal
from multiprocessing import cpu_count

from PySide6.QtCore import Qt, QThread, QMutex, Signal
from PySide6.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QApplication, QScrollArea, QLabel
from qfluentwidgets import TitleLabel, CaptionLabel, LineEdit, MessageBoxBase, PushButton

from config import cfg
from logger import log
from GUI.Messenger import GPUBenchmarkMessages
from CApi import *

class LimitLineEdit(LineEdit):
    def __init__(
        self,
        type_input: Literal["int", "float", "str"] = "int",
        min_value: int|float|None = None,
        max_value: int|float|None = None,
        default_value: int|float|str = "",
    ):
        super().__init__()
        assert type_input != "str" or min_value is None and max_value is None
        self.type_input = type_input
        self.editingFinished.connect(self._on_text_edited)
        self.min_value = min_value
        self.max_value = max_value
        self.default_value = default_value
        self.value = default_value

        self.setText(str(self.default_value))

    def _type_convert(self, text: str):
        try:
            if self.type_input == "int":
                return int(text)
            elif self.type_input == "float":
                return float(text)
            else:
                return text
        except Exception:
            return None

    def _on_text_edited(self) -> None:
        text = self.text().strip()
        if text == "":
            self.setPlaceholderText("")
            self.setText(str(self.default_value))
            self.value = self.default_value
            return

        value = self._type_convert(text)
        if value is None:
            self.setPlaceholderText("请输入有效数字")
            self.setText("")
        elif self.min_value is not None and value < self.min_value:
            self.setPlaceholderText(f"最小值为{self.min_value}")
            self.setText("")
        elif self.max_value is not None and value > self.max_value:
            self.setPlaceholderText(f"最大值为{self.max_value}")
            self.setText("")
        else:
            self.setPlaceholderText("")
            self.setText(str(value))
            self.value = value

class GPUBenchmarkThread(QThread):
    def __init__(self, parent):
        super().__init__(parent)
        self.mutex = QMutex()
        self.running = False
        self.end_flag = False

        self.cpu_thread = 8
        self.gpu_thread_start = 0
        self.gpu_thread_end = 8
        self.test_time = 1.0

    def terminate(self) -> None:
        self.end_flag = True

    def isRunning(self) -> bool:
        return self.running

    def run(self):
        try:
            if not self.mutex.try_lock():
                return
            self.running = True

            gpu_benchmark = GPUBenchmark(self.cpu_thread)
            gpu_benchmark.run()
            sleep(0.5)

            for gpu_thread in range(self.gpu_thread_start, min(self.gpu_thread_end, self.cpu_thread)+1):
                set_gpu_max_worker_c(gpu_thread)
                sleep(0.3)
                gpu_benchmark.reset()
                sleep(self.test_time)
                speed = gpu_benchmark.get_speed()
                GPUBenchmarkMessages.result.emit(gpu_thread, speed)

                if self.end_flag:
                    break
        except Exception as e:
            log.error(f"GPU benchmark failed: {e}")
        finally:
            self.mutex.unlock()
            self.end_flag = False
            self.running = False
            GPUBenchmarkMessages.end.emit()

class GPUBenchmarkMessageBox(MessageBoxBase):
    def __init__(self, parent):
        super().__init__(parent)
        self.yesButton.setText("开始测试")
        self.cancelButton.setText("关闭")
        self.test_thread = GPUBenchmarkThread(parent)

        title_label = TitleLabel("GPU性能测试", self)
        self.viewLayout.addWidget(title_label)

        cpu_layout = QHBoxLayout()
        cpu_layout.addWidget(CaptionLabel("CPU线程数:", self))
        self.cpu_thread = LimitLineEdit("int", 1, 128, cfg.config.max_thread)
        cpu_layout.addWidget(self.cpu_thread)
        self.viewLayout.addLayout(cpu_layout)

        # self.viewLayout.addWidget(CaptionLabel("GPU线程数:", self))
        gpu_layout = QHBoxLayout()
        gpu_layout.addWidget(CaptionLabel("GPU线程数:", self))
        self.gpu_thread_start = LimitLineEdit("int", 0, 128, 0)
        gpu_layout.addWidget(self.gpu_thread_start)
        gpu_layout.addWidget(CaptionLabel("至", self))
        self.gpu_thread_end = LimitLineEdit("int", 0, 128, cfg.config.max_thread)
        gpu_layout.addWidget(self.gpu_thread_end)
        self.viewLayout.addLayout(gpu_layout)

        test_time_layout = QHBoxLayout()
        test_time_layout.addWidget(CaptionLabel("测试时间:", self))
        self.test_time = LimitLineEdit("float", 0.1, 60, default_value=1.0)
        test_time_layout.addWidget(self.test_time)
        self.viewLayout.addLayout(test_time_layout)

        self.result_label = CaptionLabel(self)
        self.result_label.setWordWrap(True)
        self.result_label.setAlignment(Qt.AlignTop | Qt.AlignLeft)
        self.result_label.setTextInteractionFlags(Qt.TextSelectableByMouse)

        result_area = QScrollArea(self)
        result_area.setMinimumHeight(200)
        result_area.setWidgetResizable(True)
        result_area.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

        result_container = QWidget()
        result_layout = QVBoxLayout(result_container)
        result_layout.setContentsMargins(0, 0, 0, 0)
        result_layout.addWidget(self.result_label)
        result_layout.addStretch(1)

        result_area.setWidget(result_container)
        self.viewLayout.addWidget(result_area)

        self.test_result = []
        GPUBenchmarkMessages.result.connect(self.__update_text)
        GPUBenchmarkMessages.end.connect(self.__on_test_end)

    def validate(self):
        if self.test_thread.isRunning():
            return False
        self.yesButton.setEnabled(False)

        self.test_result.clear()
        self.test_thread.cpu_thread = self.cpu_thread.value
        self.test_thread.gpu_thread_start = self.gpu_thread_start.value
        self.test_thread.gpu_thread_end = self.gpu_thread_end.value
        self.test_thread.test_time = self.test_time.value
        self.test_thread.start()
        return False

    def __on_test_end(self):
        self.yesButton.setEnabled(True)

    def __update_text(self, gpu_thread: int, speed: float):
        self.test_result.append((gpu_thread, speed))
        cur_max = max(self.test_result, key=lambda x: x[1])
        text = "\n".join(f"线程 {i[0]}: {i[1]:.1f} planets/s" for i in self.test_result)
        text += f"\n最优:\n线程 {cur_max[0]}: {cur_max[1]:.1f} planets/s"
        self.result_label.setText(text)
