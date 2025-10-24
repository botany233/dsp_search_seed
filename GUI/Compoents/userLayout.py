from PySide6.QtWidgets import QVBoxLayout, QHBoxLayout, QFileDialog
from PySide6.QtGui import QColor
from PySide6.QtCore import QTimer, Qt

from qfluentwidgets import PushButton, BodyLabel, ProgressBar, CaptionLabel, LineEdit

from config import cfg


class UserLayout(QVBoxLayout):
    def __init__(self):
        super().__init__()
        self.timestamp = 114514
        self.userButtonsLayout = QHBoxLayout()

        self.addLayout(self.userButtonsLayout)
        self.addStretch()

        self.outputFileLabel = BodyLabel("输出文件名称 ")
        self.userButtonsLayout.addWidget(self.outputFileLabel)
        self.outputFileLine = LineEdit()
        self.outputFileLine.setPlaceholderText("output")
        self.outputFileLine.textEdited.connect(self._update_output_filename)
        self.userButtonsLayout.addWidget(self.outputFileLine)

        self.barVLayout = QVBoxLayout()
        self.barLabel = CaptionLabel("搜索进度")
        self.barVLayout.addWidget(self.barLabel)
        self.barVLayout.setAlignment(self.barLabel, Qt.AlignRight)
        self.progressBar = ProgressBar()
        self.progressBar.setValue(50)
        self.progressBar.setFixedHeight(6)
        self.barVLayout.addWidget(self.progressBar)
        self.userButtonsLayout.addLayout(self.barVLayout)
        self.userButtonsLayout.setStretchFactor(self.barVLayout, 2)

        self.setContentsMargins(7, 0, 7, 7)

        self.timer = QTimer()
        self.timer.setInterval(100)  # 100ms
        self.timer.timeout.connect(self._update_progress)
        self.timer.start()

    def _update_output_filename(self, filename: str):
        if filename == "":
            filename = "output.csv"
        cfg.output_file_name = filename + ".csv"

    def _update_progress(self):
        # 这里可以添加更新进度条的逻辑
        current_value = self.progressBar.value()
        new_value = (current_value + 1) % 101  # 循环从0到100
        self.progressBar.setValue(new_value)
        self.barLabel.setText(f"搜索进度: {new_value}%  " + self.time_format(int(self.timestamp * (100 - new_value) / 100)))

    def time_format(self, timestamp: int) -> str:
        hours = timestamp // 3600
        minutes = (timestamp % 3600) // 60
        seconds = timestamp % 60
        return f"还有{hours:02}小时{minutes:02}分{seconds:02}秒"