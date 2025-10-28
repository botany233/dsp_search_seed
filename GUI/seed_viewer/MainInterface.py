from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog
from PySide6.QtCore import Qt
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, TableWidget, TableItemDelegate
from csv import reader
from .Compoents import *

class ViewerInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.seed_list = []
        self.max_seed = 100000

        qss = """
        QFrame {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 4px;
        }
        """
        self.setStyleSheet(qss)

        self.mainLayout = QHBoxLayout(self)
        
        self.__init_left()

        self.middleLayout = QVBoxLayout()
        self.rightLayout = QVBoxLayout()
        
        self.mainLayout.addLayout(self.middleLayout)
        self.mainLayout.addLayout(self.rightLayout)

        middle = TitleLabel("中间区域")
        middle.setAlignment(Qt.AlignCenter)
        right = TitleLabel("右侧区域")
        right.setAlignment(Qt.AlignCenter)

        self.middleLayout.addWidget(middle)
        self.rightLayout.addWidget(right)

    def __init_left(self):
        self.leftWidget = QWidget()
        self.leftWidget.setFixedWidth(300)
        self.mainLayout.addWidget(self.leftWidget)
        self.leftLayout = QVBoxLayout(self.leftWidget)

        # self.seed_scroll = ListWidget()
        # self.seed_scroll.setEditTriggers(ListWidget.NoEditTriggers)
        # self.seed_scroll.setItemDelegate(ListItemDelegate(self.seed_scroll))
        # self.leftLayout.addWidget(self.seed_scroll)

        self.seed_scroll = SeedScroll()
        self.leftLayout.addWidget(self.seed_scroll)

        self.seed_text = BodyLabel()
        self.seed_text.setAlignment(Qt.AlignCenter)
        self.seed_text.setFixedHeight(30)
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

    def refresh_seed_text(self) -> None:
        self.seed_text.setText(f"种子数: {len(self.seed_list)}")

    def __on_delete_button_clicked(self) -> None:
        pass

    def __on_seed_button_clicked(self) -> None:
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            '选择CSV文件',
            '',
            'CSV Files (*.csv);;All Files (*)'
        )

        if not file_path or not file_path.endswith(".csv"):
            return

        with open(file_path, "r", encoding="utf-8") as f:
            data = reader(f)
            for row in data:
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
