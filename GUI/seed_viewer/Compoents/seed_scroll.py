from PySide6.QtWidgets import QTableWidgetItem, QApplication, QTableWidget
from PySide6.QtCore import QObject, Qt, Signal
from qfluentwidgets import TableWidget, TableItemDelegate, RoundMenu, CheckableMenu, Action, FluentIcon, MenuIndicatorType
from logger import log

class SeedScroll(TableWidget):

    SelectModeChanged = Signal(bool)
    SeedListUpdated = Signal()

    def __init__(self, seed_list: list[int, int, float|int]):
        super().__init__()
        self._multi_select = False
        self.all_selected = False
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
        self.setSelectionMode(QTableWidget.SingleSelection)

    @property
    def multi_select(self) -> bool:
        return self._multi_select

    @multi_select.setter
    def multi_select(self, value: bool):
        self._multi_select = value
        if value:
            self.setSelectionMode(QTableWidget.ExtendedSelection)
        else:
            self.setSelectionMode(QTableWidget.SingleSelection)
        self.SelectModeChanged.emit(value)

    def __on_menu_requested(self, pos):

        menu = CheckableMenu(indicatorType=MenuIndicatorType.CHECK)
        if self.disable_context_menu:
            menu.addAction(Action("搜索中, 禁用菜单"))
            menu.exec(self.viewport().mapToGlobal(pos))
            menu.closedSignal.connect(menu.deleteLater)
            return
        mult_select_action = Action("选择模式", checkable=True)
        mult_select_action.setChecked(self.multi_select)
        mult_select_action.triggered.connect(lambda _: self._switch_select_mode(pos))
        menu.addAction(mult_select_action)

        normal_action = Action("查看模式", checkable=True)
        normal_action.setChecked(not self.multi_select)
        normal_action.triggered.connect(self._switch_select_mode)
        menu.addAction(normal_action)

        if self.multi_select:
            menu.addSeparator()
            output_csv_action = Action("导出选中", triggered=self._export_select)
            menu.addAction(output_csv_action)

            select_all_action = Action("全选" if not self.all_selected else "取消全选")
            select_all_action.triggered.connect(self._select_all)
            menu.addAction(select_all_action)
        
            if self.selectedItems() and not self.all_selected:
                clear_select_action = Action("取消选中")
                clear_select_action.triggered.connect(self._clear_select)
                menu.addAction(clear_select_action)
            menu.addSeparator()
            
            del_action = Action("删除选中", triggered=self.delete_select)
            menu.addAction(del_action)


        test_action = Action("打印选中")
        test_action.triggered.connect(self.test)
        menu.addAction(test_action)

        menu.exec(self.viewport().mapToGlobal(pos))
        menu.closedSignal.connect(menu.deleteLater)

    def test(self):
        items = self.selectedItems()
        ret = set()
        assert len(items) % 3 == 0
        for seed, star_num, sort_value in zip(items[::3], items[1::3], items[2::3]):
            ret.add((int(seed.text()), int(star_num.text())))
        print(ret)

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

    def _export_select(self) -> None:
        data = self.get_select_seed()
        if data:
            for (seed, star_num) in data:
                print(seed, star_num)
                # TODO: 靠你惹
        log.error("靠你惹")

    def _switch_select_mode(self, pos: None) -> None:
        self.multi_select = not self.multi_select
        # if isinstance(pos, bool):
        #     pos = None

        # if self.multi_select and pos:
        #     self.__on_menu_requested(pos)

    def _select_all(self) -> None:
        if not self.multi_select:
            return
        
        if self.all_selected:
            self._clear_select()
        else:
            self.selectAll()
            self.all_selected = True
    
    def _clear_select(self) -> None:
        if not self.multi_select:
            return
        
        self.clearSelection()
        self.all_selected = False

    def delete_select(self) -> None:
        data = self.get_select_seed(True)
        for (seed, star_num) in data:
            for index, (s, sn, _) in enumerate(self.seed_list):
                if s == seed and sn == star_num:
                    self.seed_list.pop(index)
                    break
        self.SeedListUpdated.emit()
            

    def get_select_seed(self, pop = False) -> set[tuple[int, int]]:
        """
        Returns:
            ret: set (seed, star_num)
        """
        items = self.selectedItems()
        data = set()
        assert len(items) % 3 == 0
        for seed, star_num, sort_value in zip(items[::3], items[1::3], items[2::3]):
            data.add((int(seed.text()), int(star_num.text())))
            if pop:
                self.removeRow(seed.row())
        if pop:
            self.clearSelection()
        return data

    def get_table_value(self) -> list[tuple[int, int, float]]:
        row_count = self.rowCount()
        data = []
        for row in range(row_count):
            data.append((int(self.item(row, 0).text()), int(self.item(row, 1).text()), float(self.item(row, 2).text())))
        return data
