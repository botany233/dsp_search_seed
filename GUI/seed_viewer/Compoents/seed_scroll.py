from PySide6.QtWidgets import QTableWidgetItem, QApplication
from qfluentwidgets import TableWidget, TableItemDelegate

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

    def add_row(self, seed: int, star_num: int) -> None:
        row_count = self.rowCount()
        self.setRowCount(row_count + 1)
        self.setItem(row_count, 0, QTableWidgetItem(str(seed)))
        self.setItem(row_count, 1, QTableWidgetItem(str(star_num)))
        self.setItem(row_count, 2, QTableWidgetItem(str(0)))

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
        target_seed, target_star_num = self.get_select_seed(True)
        for i, (seed, star_num, _) in enumerate(self.seed_list):
            if seed == target_seed and star_num == target_star_num:
                self.seed_list.pop(i)
                return

    def get_select_seed(self, pop = False) -> tuple[int, int]:
        selected_indexes = self.selectedIndexes()
        if not selected_indexes:
            return -1, -1
        row = selected_indexes[0].row()
        seed = int(self.item(row, 0).text())
        star_num = int(self.item(row, 1).text())
        if pop:
            self.removeRow(row)
        return seed, star_num

    def get_table_value(self) -> list[tuple[int, int, float]]:
        row_count = self.rowCount()
        data = []
        for row in range(row_count):
            data.append((int(self.item(row, 0).text()), int(self.item(row, 1).text()), float(self.item(row, 2).text())))
        return data
