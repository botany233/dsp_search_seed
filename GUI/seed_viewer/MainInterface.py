from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QApplication, QGridLayout
from PySide6.QtCore import Qt
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, CaptionLabel
from csv import reader
from .Compoents import *
from threading import Lock
from .sort_seed import SortThread

class ViewerInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.seed_list = []
        self.seed_list_lock = Lock()
        self.max_seed = 100000

        self.mainLayout = QHBoxLayout(self)
        self.__init_left()
        self.__init_middle()
        self.__init_right()
        self.mainLayout.setStretch(0, 1)
        self.mainLayout.setStretch(1, 2)
        self.mainLayout.setStretch(2, 1)

        self.sort_thread = SortThread(self)

    def __init_left(self):
        self.leftWidget = QWidget()
        self.leftWidget.setFixedWidth(263)
        self.mainLayout.addWidget(self.leftWidget)
        self.leftLayout = QVBoxLayout(self.leftWidget)

        self.leftLayout.setContentsMargins(0, 0, 0, 0)

        self.seed_scroll = SeedScroll(self.seed_list)
        self.leftLayout.addWidget(self.seed_scroll)
        self.seed_scroll.itemSelectionChanged.connect(self.__on_select_seed_change)

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

        self.update_seed_text()

    def __init_middle(self):
        self.middleLayout = QVBoxLayout()
        self.mainLayout.addLayout(self.middleLayout)

        middle = TitleLabel("中间区域")
        middle.setAlignment(Qt.AlignCenter)
        self.middleLayout.addWidget(middle)

    def __init_right(self):
        self.rightLayout = QVBoxLayout()
        self.mainLayout.addLayout(self.rightLayout)

        self.infoLayout = QVBoxLayout()

        self.infoLayout.addWidget(TitleLabel("信息区域"), alignment=Qt.AlignCenter)

        self.buttonsLayout = QGridLayout()

        self.main_type_combo = MainTypeComboBox()
        self.sub_type_combo = SubTypeComboBox(self.main_type_combo)

        self.sort_order_switch = SortOrderSwitch()
        # self.sort_order_switch.checkedChanged.connect(self.__on_sort_order_clicked)

        self.progress_label = BodyLabel("进度: 0/0 (0%)")

        self.start_button = PushButton("开始搜索")
        self.stop_button = PushButton("停止搜索")
        self.stop_button.setEnabled(False)

        self.rightLayout.addLayout(self.infoLayout)
        self.rightLayout.addStretch()
        self.rightLayout.addLayout(self.buttonsLayout)

        self.buttonsLayout.addWidget(self.main_type_combo, 0, 0)
        self.buttonsLayout.addWidget(self.sub_type_combo, 0, 1)
        self.buttonsLayout.addWidget(self.sort_order_switch, 1, 0)
        self.buttonsLayout.addWidget(self.progress_label, 1, 1)
        self.buttonsLayout.addWidget(self.start_button, 2, 0)
        self.buttonsLayout.addWidget(self.stop_button, 2, 1)

    def __on_select_seed_change(self):
        seed, star_num = self.seed_scroll.get_select_seed()
        print("已选中：", seed, star_num)

    def update_seed_text(self) -> None:
        self.seed_text.setText(f"种子数: {len(self.seed_list)}")

    # def __on_sort_order_clicked(self, checked: bool):
    #     if checked:
    #         qss = """
    #         QFrame {
    #             border: 1px solid #ccc;
    #             border-radius: 4px;
    #             padding: 4px;
    #         }
    #         """
    #     else:
    #         qss = ""

    #     self.setStyleSheet(qss)

    def __on_delete_button_clicked(self) -> None:
        try:
            if not self.seed_list_lock.acquire(False):
                return
            self.seed_scroll.delete_select()
        finally:
            self.seed_list_lock.release()

    def __on_seed_button_clicked(self) -> None:
        try:
            if not self.seed_list_lock.acquire(False):
                return

            file_paths, _ = QFileDialog.getOpenFileNames(
                self,
                '选择CSV文件',
                '',
                'CSV Files (*.csv);'
            )

            if not file_paths:
                return

            if len(self.seed_list) >= self.max_seed:
                return

            seed_set = set([(i[0], i[1]) for i in self.seed_list])
            for file_path in file_paths:
                QApplication.processEvents()
                if len(self.seed_list) >= self.max_seed:
                    break
                with open(file_path, "r", encoding="utf-8") as f:
                    data = reader(f)
                    for row in data:
                        if len(self.seed_list) >= self.max_seed:
                            break
                        try:
                            seed = int(row[0])
                            star_num = int(row[1])
                        except Exception:
                            continue

                        if not (0 <= seed <= 99999999 and 32 <= star_num <= 64):
                            continue

                        if (seed, star_num) not in seed_set:
                            seed_set.add((seed, star_num))
                            self.seed_list.append([seed, star_num, 0])

            self.update_seed_text()
            self.seed_scroll.fresh()
        finally:
            self.seed_list_lock.release()
