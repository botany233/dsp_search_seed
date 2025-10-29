from PySide6.QtWidgets import QTableWidgetItem, QApplication
from qfluentwidgets import TableWidget, TableItemDelegate
from PySide6.QtCore import Qt

class SeedScroll(TableWidget):
    def __init__(self, seed_list: list[int, int, float|int]):
        super().__init__()
        self.seed_list = seed_list
        self.setEditTriggers(TableWidget.NoEditTriggers)
        self.setItemDelegate(TableItemDelegate(self))
        self.setColumnCount(3)
        self.setHorizontalHeaderLabels(["种子", "恒星数", "排序值"])
        self.verticalHeader().hide()
        self.setColumnWidth(0, 105)
        self.setColumnWidth(1, 60)
        self.setColumnWidth(2, 90)
        self.setBorderVisible(True)
        self.setBorderRadius(8)
        header = self.horizontalHeader()
        header.setSectionResizeMode(0, header.ResizeMode.Fixed)
        header.setSectionResizeMode(1, header.ResizeMode.Fixed)
        header.setSectionResizeMode(2, header.ResizeMode.Fixed)
        self.setSelectionMode(TableWidget.SingleSelection)

    def fresh(self) -> None:
        self.setRowCount(len(self.seed_list))
        for row in range(len(self.seed_list)):
            for col in range(3):
                self.setItem(row, col, QTableWidgetItem(str(self.seed_list[row][col])))

    def do_sort(self, is_ascending_order = True) -> None:
        sort_list = [(i, value[2]) for i, value in enumerate(self.seed_list)]
        if is_ascending_order:
            sort_list.sort(key=lambda x:x[1])
        else:
            sort_list.sort(key=lambda x:x[1], reverse=True)
        for table_row, i in enumerate(sort_list):
            QApplication.processEvents()
            seed_row = i[0]
            for table_col in range(3):
                self.setItem(table_row, table_col, QTableWidgetItem(str(self.seed_list[seed_row][table_col])))

    def delete_select(self) -> None:
        target_seed, target_star_num = self.get_select_seed()
        for i, (seed, star_num, _) in enumerate(self.seed_list):
            QApplication.processEvents()
            if seed == target_seed and star_num == target_star_num:
                self.seed_list.pop(i)
                break
        self.fresh()

    def get_select_seed(self) -> tuple[int, int]:
        selected_items = self.selectedItems()
        seed = int(selected_items[0].text())
        star_num = int(selected_items[1].text())
        return seed, star_num
