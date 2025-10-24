from PySide6.QtWidgets import QVBoxLayout, QHBoxLayout, QFileDialog
from PySide6.QtGui import QColor
from PySide6.QtCore import QTimer, Qt

from qfluentwidgets import PushButton, BodyLabel, ProgressBar, CaptionLabel, LineEdit

from config import cfg

from GUI.Messenger import SearchMessages

class UserLayout(QVBoxLayout):
    def __init__(self):
        super().__init__()
        self.timestamp = 114514
        self.seeds = (1, 1)
        self.batch_size = 1

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

        SearchMessages.search_last_seed.connect(self._update_seed_info)
        SearchMessages.search_progress.connect(self._update_progress)
        SearchMessages.searchEndNormal.connect(self._on_search_finish)

    def _update_output_filename(self, filename: str):
        if filename == "":
            filename = "output.csv"
        cfg.output_file_name = filename + ".csv"

    def _update_seed_info(self, seed: int, star_num: int = 0, founded_seeds: int = 0):
        msg = f"符合条件的种子数量: {founded_seeds}  上次命中的种子: {seed}  恒星数: {star_num}"
        self.seedInfoLabel.setText(msg)

    def _on_search_finish(self):
        self.barLabel.setText("搜索完成！")
        self.progressBar.setValue(self.progressBar.maximum())

    def _update_progress(self, value: int):
        self.progressBar.setValue(value)
        self.barLabel.setText(
            f"搜索进度: {value * 100 // self.progressBar.maximum()}%  "
            + self.time_format(
                int(
                    self.timestamp
                    * (self.progressBar.maximum() - value)
                    / self.progressBar.maximum()
                )
            )
        )

    def time_format(self, timestamp: int) -> str:
        hours = timestamp // 3600
        minutes = (timestamp % 3600) // 60
        seconds = timestamp % 60
        return f"还有{hours:02}小时{minutes:02}分{seconds:02}秒"