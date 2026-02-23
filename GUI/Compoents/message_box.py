from time import sleep
from typing import Literal
from multiprocessing import cpu_count

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QWidget, QGridLayout, QHBoxLayout, QVBoxLayout, QApplication, QScrollArea, QLabel
from qfluentwidgets import TitleLabel, CaptionLabel, LineEdit, MessageBoxBase, ScrollArea, isDarkTheme

from config import cfg
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

class GPUBenchmarkMessageBox(MessageBoxBase):
    def __init__(self, parent):
        super().__init__(parent)
        self.yesButton.setText("开始测试")
        self.cancelButton.setText("关闭")

        title_label = TitleLabel("GPU性能测试")
        self.viewLayout.addWidget(title_label)

        cpu_layout = QHBoxLayout()
        cpu_layout.addWidget(CaptionLabel("CPU线程数:"))
        self.cpu_thread = LimitLineEdit("int", 1, 128, cfg.config.max_thread)
        cpu_layout.addWidget(self.cpu_thread)
        self.viewLayout.addLayout(cpu_layout)

        # self.viewLayout.addWidget(CaptionLabel("GPU线程数:", self))
        gpu_layout = QHBoxLayout()
        gpu_layout.addWidget(CaptionLabel("GPU线程数:"))
        self.gpu_thread_start = LimitLineEdit("int", 0, 128, 0)
        gpu_layout.addWidget(self.gpu_thread_start)
        gpu_layout.addWidget(CaptionLabel("至", self))
        self.gpu_thread_end = LimitLineEdit("int", 0, 128, cfg.config.max_thread)
        gpu_layout.addWidget(self.gpu_thread_end)
        self.viewLayout.addLayout(gpu_layout)

        test_time_layout = QHBoxLayout()
        test_time_layout.addWidget(CaptionLabel("测试时间:"))
        self.test_time = LimitLineEdit("float", 0.1, 60, default_value=1.0)
        test_time_layout.addWidget(self.test_time)
        self.viewLayout.addLayout(test_time_layout)

        self.result_label = CaptionLabel()
        self.result_label.setWordWrap(True)
        self.result_label.setAlignment(Qt.AlignTop | Qt.AlignLeft)
        self.result_label.setTextInteractionFlags(Qt.TextSelectableByMouse)

        result_area = ScrollArea()
        result_area.setMinimumHeight(200)
        result_area.setWidgetResizable(True)
        result_area.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        qss = """
QScrollArea {
    border: 1px solid --border-color;
    border-radius: 4px;
}
QScrollArea > QWidget > QLabel {
    padding-left: 8px;
}"""
        result_area.setStyleSheet(qss.replace("--border-color", "#ededed" if not isDarkTheme() else "#4B4B4B"))

        result_area.setWidget(self.result_label)
        self.viewLayout.addWidget(result_area)

        self.testing = False

    def validate(self):
        if self.testing:
            return False
        self.testing = True
        self.cancelButton.setEnabled(False)
        cpu_thread = self.cpu_thread.value

        gpu_benchmark = GPUBenchmark(cpu_thread)
        gpu_benchmark.run()

        results = []
        for gpu_thread in range(self.gpu_thread_start.value, min(self.gpu_thread_end.value, cpu_thread)+1):
            set_gpu_max_worker_c(gpu_thread)
            sleep(0.5)
            QApplication.processEvents()
            speed = gpu_benchmark.do_test(self.test_time.value)
            results.append(f"thread {gpu_thread}: {speed:.1f}planet/s")
            self.result_label.setText("\n".join(results))
            QApplication.processEvents()

        self.testing = False
        self.cancelButton.setEnabled(True)
        return False
