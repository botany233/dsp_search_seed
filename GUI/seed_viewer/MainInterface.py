from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QGridLayout, QTreeWidgetItem, QApplication
from PySide6.QtCore import Qt
from PySide6.QtWidgets import QDialog
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, CaptionLabel
from datetime import datetime
from csv import reader
from .Compoents import *
from .sort_seed import SortThread
from CApi import get_galaxy_data_c

class ViewerInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.seed_list = []
        self.max_seed = 100000

        self.mainLayout = QHBoxLayout(self)
        self.__init_left()
        self.__init_middle()
        self.__init_right()
        self.mainLayout.setStretch(0, 1)
        self.mainLayout.setStretch(1, 2)
        self.mainLayout.setStretch(2, 1)

        self.sort_thread = SortThread(self)
        self.sort_thread.label_text.connect(self.progress_label.setText)
        self.sort_thread.completed.connect(self.__on_sort_completed)
        self.sort_thread.finished.connect(self.__on_sort_finished)

    def __init_left(self):
        self.leftWidget = QWidget()
        self.leftWidget.setFixedWidth(263)
        self.mainLayout.addWidget(self.leftWidget)
        self.leftLayout = QVBoxLayout(self.leftWidget)

        self.leftLayout.setContentsMargins(0, 0, 0, 0)

        self.seed_scroll = SeedScroll(self.seed_list)
        self.leftLayout.addWidget(self.seed_scroll)
        self.seed_scroll.itemClicked.connect(self.__on_select_seed_change)

        self.seed_text = SeedText(self.seed_list, self.max_seed)
        self.seed_text.setAlignment(Qt.AlignBottom)
        self.leftLayout.addWidget(self.seed_text)

        self.seed_table_button_layout = QHBoxLayout()

        self.delete_button = PushButton("删除选中种子")
        self.delete_button.setFixedHeight(30)
        self.delete_button.clicked.connect(self.__on_delete_button_clicked)
        self.seed_table_button_layout.addWidget(self.delete_button)

        self.manual_add_button = PushButton("手动加入种子")
        self.manual_add_button.setFixedHeight(30)
        self.manual_add_button.clicked.connect(self.__on_manual_add_button_clicked)
        self.seed_table_button_layout.addWidget(self.manual_add_button)

        self.leftLayout.addLayout(self.seed_table_button_layout)

        self.seed_button_layout = QHBoxLayout()

        self.add_button = PushButton("导入种子")
        self.add_button.setFixedHeight(30)
        self.add_button.clicked.connect(self.__on_add_button_clicked)
        self.seed_button_layout.addWidget(self.add_button)

        self.export_button = PushButton("导出种子")
        self.export_button.setFixedHeight(30)
        self.export_button.clicked.connect(self.__on_export_button_clicked)
        self.seed_button_layout.addWidget(self.export_button)

        self.leftLayout.addLayout(self.seed_button_layout)

        self.seed_text.fresh()

    def __init_middle(self):
        self.astro_tree = AstroTree()
        self.astro_tree.setBorderVisible(True)
        self.astro_tree.setBorderRadius(8)
        self.astro_tree.itemClicked.connect(self.__on_select_astro)
        self.mainLayout.addWidget(self.astro_tree)

    def __init_right(self):
        self.rightLayout = QVBoxLayout()
        self.mainLayout.addLayout(self.rightLayout)

        self.astro_info = AstroInfo()

        self.buttonsLayout = QGridLayout()

        self.main_type_combo = MainTypeComboBox()
        self.sub_type_combo = SubTypeComboBox(self.main_type_combo)

        self.sort_order_switch = SortOrderSwitch()

        self.progress_label = BodyLabel("进度: 0/0 (0%)")

        self.start_button = PushButton("开始排序")
        self.start_button.clicked.connect(self.__on_start_button_clicked)
        self.stop_button = PushButton("停止排序")
        self.stop_button.clicked.connect(self.__on_stop_button_clicked)
        self.stop_button.setEnabled(False)

        self.rightLayout.addWidget(self.astro_info)
        self.rightLayout.addStretch()
        self.rightLayout.addLayout(self.buttonsLayout)

        self.buttonsLayout.addWidget(self.main_type_combo, 0, 0)
        self.buttonsLayout.addWidget(self.sub_type_combo, 0, 1)
        self.buttonsLayout.addWidget(self.sort_order_switch, 1, 0)
        self.buttonsLayout.addWidget(self.progress_label, 1, 1)
        self.buttonsLayout.addWidget(self.start_button, 2, 0)
        self.buttonsLayout.addWidget(self.stop_button, 2, 1)

    def __on_manual_add_button_clicked(self) -> None:
        dlg = ManualAddMessageBox(self.seed_list, self)
        if dlg.exec() != QDialog.Accepted:
            return

        seed_id, star_num = int(dlg.seed_id.text()), int(dlg.star_num.text())
        self.seed_list.append([seed_id, star_num, 0])
        self.seed_scroll.add_row(seed_id, star_num)
        self.seed_text.fresh()

    def __on_start_button_clicked(self) -> None:
        if self.sort_thread.isRunning():
            return
        if len(self.seed_list) < 2:
            self.progress_label.setText("<font color='red'>请导入种子，再排序！</font>")
            return
        self.start_button.setEnabled(False)
        self.stop_button.setEnabled(True)
        self.delete_button.setEnabled(False)
        self.add_button.setEnabled(False)
        self.sort_thread.start()

    def __on_stop_button_clicked(self) -> None:
        self.sort_thread.terminate()

    def __on_sort_finished(self) -> None:
        self.start_button.setEnabled(True)
        self.stop_button.setEnabled(False)
        self.delete_button.setEnabled(True)
        self.add_button.setEnabled(True)

    def __on_sort_completed(self) -> None:
        self.progress_label.setText("排序中...")
        self.seed_scroll.do_sort(self.sort_order_switch.isChecked())
        self.progress_label.setText("排序完成！")

    def __on_select_seed_change(self):
        seed, star_num = self.seed_scroll.get_select_seed()
        if seed < 0 or star_num < 0:
            return
        galaxy_data = get_galaxy_data_c(seed, star_num)
        self.astro_tree.fresh(galaxy_data)

    def __on_select_astro(self, item: QTreeWidgetItem, column: int) -> None:
        self.astro_info.fresh(item)

    def __on_delete_button_clicked(self) -> None:
        if self.sort_thread.isRunning():
            return
        self.seed_scroll.delete_select()
        self.seed_text.fresh()

    def __on_export_button_clicked(self) -> None:
        if self.sort_thread.isRunning():
            return

        file_path, _ = QFileDialog.getSaveFileName(
            self,
            "保存文件",
            "export_seed",
            "CSV Files (*.csv);"
        )

        if not file_path:
            return

        table_value = self.seed_scroll.get_table_value()
        QApplication.processEvents()
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(f"seed_id, star_num, value\n")
            f.writelines(f"{i[0]}, {i[1]}, {i[2]}\n" for i in table_value)

    def __on_add_button_clicked(self) -> None:
        if self.sort_thread.isRunning():
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
                        self.seed_scroll.add_row(seed, star_num)

        self.seed_text.fresh()
