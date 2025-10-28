from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QApplication, QGridLayout
from PySide6.QtCore import Qt
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, TableWidget, TableItemDelegate, CaptionLabel
from csv import reader
from .Compoents import *

from GUI.Compoents import AutoFixedComboBox, ConfigSwitchButton


class ViewerInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.seed_list = []
        self.max_seed = 100


        self.mainLayout = QHBoxLayout(self)
        
        self.__init_left()
        self.__init_middle()
        self.__init_right()

        middle = TitleLabel("中间区域")
        middle.setAlignment(Qt.AlignCenter)

        self.middleLayout.addWidget(middle)


        self.mainLayout.setStretch(0, 1)
        self.mainLayout.setStretch(1, 2)
        self.mainLayout.setStretch(2, 1)


    def __init_left(self):
        self.leftWidget = QWidget()
        self.leftWidget.setFixedWidth(263)
        self.mainLayout.addWidget(self.leftWidget)
        self.leftLayout = QVBoxLayout(self.leftWidget)

        self.leftLayout.setContentsMargins(0, 0, 0, 0)

        # self.seed_scroll = ListWidget()
        # self.seed_scroll.setEditTriggers(ListWidget.NoEditTriggers)
        # self.seed_scroll.setItemDelegate(ListItemDelegate(self.seed_scroll))
        # self.leftLayout.addWidget(self.seed_scroll)

        self.seed_scroll = SeedScroll()
        self.leftLayout.addWidget(self.seed_scroll)

        self.seed_text = CaptionLabel()
        self.seed_text.setAlignment(Qt.AlignBottom)
        self.leftLayout.addWidget(self.seed_text)

        self.delete_button = PushButton("删除选中种子")
        self.delete_button.setFixedHeight(30)
        self.delete_button.clicked.connect(self.__on_delete_button_clicked)
        self.leftLayout.addWidget(self.delete_button)

        self.seed_button = PushButton("导入种子")
        self.seed_button.setFixedHeight(30)
        self.seed_button.clicked.connect(self.__on_seed_button_clicked)
        self.leftLayout.addWidget(self.seed_button)

        self.refresh_seed_text()

    def __init_middle(self):
        self.middleLayout = QVBoxLayout()
        self.mainLayout.addLayout(self.middleLayout)

    def __init_right(self):
        self.rightLayout = QVBoxLayout()
        self.mainLayout.addLayout(self.rightLayout)

        self.infoLayout = QVBoxLayout()

        self.infoLayout.addWidget(TitleLabel("信息区域"), alignment=Qt.AlignCenter)

        self.buttonsLayout = QGridLayout()

        self.mainSortCombo = AutoFixedComboBox()

        self.subSortCombo = AutoFixedComboBox()

        self.sortOrderSwitch = ConfigSwitchButton()
        self.sortOrderSwitch.checkedChanged.connect(self.__on_sort_order_clicked)

        self.progressLabel = BodyLabel("进度: 0/0 (0%)")

        self.startButon = PushButton("开始搜索")

        self.stopButton = PushButton("停止搜索")

        self.rightLayout.addLayout(self.infoLayout)
        self.rightLayout.addStretch()
        self.rightLayout.addLayout(self.buttonsLayout)

        self.buttonsLayout.addWidget(self.mainSortCombo, 0, 0)
        self.buttonsLayout.addWidget(self.subSortCombo, 0, 1)
        self.buttonsLayout.addWidget(self.sortOrderSwitch, 1, 0)
        self.buttonsLayout.addWidget(self.progressLabel, 1, 1)
        self.buttonsLayout.addWidget(self.startButon, 2, 0)
        self.buttonsLayout.addWidget(self.stopButton, 2, 1)



    def refresh_seed_text(self) -> None:
        self.seed_text.setText(f"种子数: {len(self.seed_list)}")

    def __on_sort_order_clicked(self, checked: bool):
        if checked:
            qss = """
            QFrame {
                border: 1px solid #ccc;
                border-radius: 4px;
                padding: 4px;
            }
            """
        else:
            qss = ""

        self.setStyleSheet(qss)

    def __on_delete_button_clicked(self) -> None:
        pass

    def __on_seed_button_clicked(self) -> None:
        file_paths, _ = QFileDialog.getOpenFileNames(
            self,
            '选择CSV文件',
            '',
            'CSV Files (*.csv);'
        )

        if not file_paths:
            return
    
        for file_path in file_paths:

            with open(file_path, "r", encoding="utf-8") as f:
                data = reader(f)
                for row in data:
                    QApplication.processEvents()
                    try:
                        seed = int(row[0])
                        star_num = int(row[1])
                    except Exception:
                        continue

                    if not (0 <= seed <= 99999999 and 32 <= star_num <= 64):
                        continue
                    if len(self.seed_list) < self.max_seed and (seed, star_num) not in self.seed_list:
                        self.seed_list.append((seed, star_num, 0))

        self.refresh_seed_text()
        self.seed_scroll.update(self.seed_list)
