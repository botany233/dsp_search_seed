from PySide6.QtWidgets import QTableWidgetItem
from PySide6.QtCore import Qt
from qfluentwidgets import TableWidget, TableItemDelegate

class SeedScroll(TableWidget):
    def __init__(self):
        super().__init__()
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

    def update(self, seed_list: list[int, int, float|int]) -> None:
        self.setRowCount(len(seed_list))
        for row in range(len(seed_list)):
            for col in range(3):
                self.setItem(row, col, QTableWidgetItem(str(seed_list[row][col])))