from PySide6.QtWidgets import QTableWidgetItem, QApplication, QTableWidget
from PySide6.QtCore import QObject, Qt, Signal, QMutex, QMutexLocker
from qfluentwidgets import TableWidget, TableItemDelegate, RoundMenu, CheckableMenu, Action, FluentIcon, MenuIndicatorType
from logger import log

from .seed_list import SeedList
from .export_window import ExportWindow

class SeedScroll(TableWidget):
    SeedListUpdated = Signal()
    def __init__(self, seed_list: SeedList):
        super().__init__()
        self.mutex = QMutex()

        self.selected_num = 0
        self.disable_context_menu = False
        self.seed_list = seed_list
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self.__on_menu_requested)
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
        self.setSelectionMode(QTableWidget.ExtendedSelection)

    #     self.itemSelectionChanged.connect(self.__update_selected_num)

    # def __update_selected_num(self):
    #     self.selected_num = len(self.selectedItems()) // 3

    def __on_menu_requested(self, pos):
        menu = RoundMenu()
        if self.disable_context_menu:
            menu.addAction(Action("搜索中, 禁用菜单"))
            menu.exec(self.viewport().mapToGlobal(pos))
            menu.closedSignal.connect(menu.deleteLater)
            return
        output_csv_action = Action("导出选中种子", triggered=self._export_select)
        menu.addAction(output_csv_action)

        # select_all_action = Action("全选" if not self.all_selected else "取消全选")
        # select_all_action.triggered.connect(self._select_all)
        # menu.addAction(select_all_action)

        # if self.selectedItems() and not self.all_selected:
        #     clear_select_action = Action("取消选中")
        #     clear_select_action.triggered.connect(self._clear_select)
        #     menu.addAction(clear_select_action)
        # menu.addSeparator()

        # del_action = Action("删除选中", triggered=self.delete_select)
        # menu.addAction(del_action)

        # test_action = Action("打印选中")
        # test_action.triggered.connect(self.test)
        # menu.addAction(test_action)

        menu.exec(self.viewport().mapToGlobal(pos))
        menu.closedSignal.connect(menu.deleteLater)

    # @property
    # def all_selected(self) -> bool:
    #     if not self.seed_list:
    #         return False
    #     return self.selected_num == len(self.seed_list)

    # def test(self):
    #     items = self.selectedItems()
    #     ret = set()
    #     assert len(items) % 3 == 0
    #     for seed, star_num, sort_value in zip(items[::3], items[1::3], items[2::3]):
    #         ret.add((int(seed.text()), int(star_num.text())))
    #     print(ret)

    # def fresh(self) -> None:
    #     locker = QMutexLocker(self.mutex)
    #     data = self.seed_list.get_all_data()
    #     self.setRowCount(len(data))
    #     for row, (seed, star_num, sort_value) in enumerate(data):
    #         self.setItem(row, 0, QTableWidgetItem(str(seed)))
    #         self.setItem(row, 1, QTableWidgetItem(str(star_num)))
    #         self.setItem(row, 2, QTableWidgetItem(str(sort_value)))

    def add_row(self, seed: int, star_num: int) -> None:
        locker = QMutexLocker(self.mutex)
        row_count = self.rowCount()
        self.setRowCount(row_count + 1)
        self.setItem(row_count, 0, QTableWidgetItem(str(seed)))
        self.setItem(row_count, 1, QTableWidgetItem(str(star_num)))
        self.setItem(row_count, 2, QTableWidgetItem(str(0)))

    def do_sort(self, is_ascending_order = True) -> None:
        locker = QMutexLocker(self.mutex)
        data = self.seed_list.get_all_data()
        data.sort(key=lambda x:x[2], reverse=not is_ascending_order)

        for table_row, (seed_id, star_num, sort_value) in enumerate(data):
            self.setItem(table_row, 0, QTableWidgetItem(str(seed_id)))
            self.setItem(table_row, 1, QTableWidgetItem(str(star_num)))
            self.setItem(table_row, 2, QTableWidgetItem(str(sort_value)))
            if table_row % 100 == 0:
                QApplication.processEvents()
        self.clearSelection()

    def _export_select(self) -> None:
        # if self.selected_num <= 0:
        #     return
        selected_seeds = self.get_select_seed()
        window = ExportWindow(self.parent().parent(), selected_seeds)
        window.show()
            # data = window.data
        # data = self.get_select_seed()
        return
        if data:
            for (seed, star_num) in data:
                print(seed, star_num)
                # TODO: 靠你惹
        log.error("靠你惹")

    # def _select_all(self) -> None:
    #     if self.all_selected:
    #         self._clear_select()
    #     else:
    #         self.selectAll()

    # def _clear_select(self) -> None:
    #     self.clearSelection()

    # def delete_select(self) -> None:
    #     data = self.get_select_seed(True)
    #     for (seed, star_num) in data:
    #         for index, (s, sn, _) in enumerate(self.seed_list):
    #             if s == seed and sn == star_num:
    #                 self.seed_list.pop(index)
    #                 break
    #     self.SeedListUpdated.emit()

    def get_select_seed(self, pop = False) -> list[tuple[int, int]]:
        locker = QMutexLocker(self.mutex)
        items = self.selectedItems()
        data = []
        assert len(items) % 3 == 0
        for seed_id, star_num, sort_value in zip(items[::3], items[1::3], items[2::3]):
            data.append((int(seed_id.text()), int(star_num.text())))
            if pop:
                self.removeRow(seed_id.row())
        if pop:
            self.clearSelection()
        return data

    def get_table_value(self) -> list[tuple[int, int, float]]:
        row_count = self.rowCount()
        data = []
        for row in range(row_count):
            data.append((int(self.item(row, 0).text()), int(self.item(row, 1).text()), float(self.item(row, 2).text())))
        return data
