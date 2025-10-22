
from qfluentwidgets.components.widgets.frameless_window import FramelessWindow
from qframelesswindow import StandardTitleBar
from qfluentwidgets import Pivot, setThemeColor, ProgressRing, setTheme, Theme
from PySide6.QtGui import QIcon, QFont, QColor, QCursor
from PySide6.QtCore import Qt
from PySide6.QtWidgets import QHBoxLayout, QGridLayout, QToolTip, QTreeWidgetItem, QSizePolicy
from qfluentwidgets import VBoxLayout, PushButton, ComboBox, TextEdit, BodyLabel, FlowLayout

from .Compoents import LabelWithComboBox
from .Widgets import SortTreeWidget
from .Compoents.Widgets.line_edit import LabelWithLineEdit, ConfigLineEdit

from GUI import vein_names, liquid, planet_types, star_types, singularity

from config import cfg

import signal

class MainWindow(FramelessWindow):


    def __init__(self):
        super().__init__()
        self.setObjectName("MainWindow")

        setThemeColor("#409EFF")
        self.setFont(QFont("微软雅黑", 12))
        title_bar = StandardTitleBar(self)

        self.setTitleBar(title_bar)
        self.setWindowTitle("戴森球种子搜索器")
        self.setWindowIcon(QIcon(r"assets\icon.png"))
        width: int = 1220
        height: int = width // 16 * 10
        self.resize(width, height)

        
        self.mainLayout = VBoxLayout(self)
        
        self.setLayout(self.mainLayout)
        self.__build__()
        #self.show()
        signal.signal(signal.SIGINT, self.__handle_exit__)

    def __init__layout__(self):
        
        self.topLayout = QGridLayout()

        self.mainLayout.addLayout(self.topLayout)
        
        self.middleLayout = QHBoxLayout()

        self.mainLayout.addLayout(self.middleLayout, stretch=1)

        self.buttonLayout = QHBoxLayout()

        self.mainLayout.addStretch()

        self.mainLayout.addLayout(self.buttonLayout)

        self.mainLayout.setContentsMargins(10, 50, 10, 10)
        pass

    def __init__widgets__(self):

        self.label_seed_range = BodyLabel("种子范围:")
        self.input_seed_start = ConfigLineEdit(config_key="seed_range[0]")
        # self.input_seed_start.setMinimumWidth(80)
        self.input_seed_end = ConfigLineEdit(config_key="seed_range[1]")
        # self.input_seed_end.setMinimumWidth(80)
        self.seed_step_range = LabelWithLineEdit("步长", config_key="step_size")
        self.label_star_num = BodyLabel("恒星数:")
        self.input_star_num_start = ConfigLineEdit(config_key="star_num_range[0]")
        self.input_star_num_end = ConfigLineEdit(config_key="star_num_range[1]")
        self.label_batch_size = BodyLabel("批处理大小:")
        self.input_batch_size = ConfigLineEdit(config_key="batch_size")
        self.label_thread_num = BodyLabel("线程数:")
        self.input_thread_num = ConfigLineEdit(config_key="max_thread")
        self.button_start = PushButton("开始搜索")
        self.button_stop = PushButton("停止搜索")

        self.tree_view = SortTreeWidget()

        copy_cfg = cfg.config.model_copy()

        for condition_config in copy_cfg.conditions:
            # ConditionConfig 层级
            condition_leaf = self.tree_view.tree.addLeaf()
            
            galaxy_condition = condition_config.galaxy_condition
            if not galaxy_condition:
                continue
            
            # GalaxyCondition 层级
            galaxy_leaf = condition_leaf.addLeaf()
            
            # 处理StarSystemConditions
            for star_system_condition in galaxy_condition.star_system_conditions:
                # StarSystemCondition 层级
                star_system_leaf = galaxy_leaf.addLeaf()
                
                # 处理PlanetConditions
                for planet_condition in star_system_condition.planet_conditions:
                    # PlanetCondition 层级
                    star_system_leaf.addLeaf()


        


    def __build__(self):
        self.__init__layout__()
        self.__init__widgets__()

        self.topLayout.addWidget(self.label_seed_range, 0, 0)
        self.topLayout.addWidget(self.input_seed_start, 0, 1)
        self.topLayout.addWidget(BodyLabel("至"), 0, 2)
        self.topLayout.addWidget(self.input_seed_end, 0, 3)
        self.topLayout.addWidget(self.seed_step_range, 0, 4)
        self.topLayout.addWidget(self.label_star_num, 0, 5)
        self.topLayout.addWidget(self.input_star_num_start, 0, 6)
        self.topLayout.addWidget(BodyLabel("至"), 0, 7)
        self.topLayout.addWidget(self.input_star_num_end, 0, 8)
        self.topLayout.addWidget(self.label_batch_size, 0, 9)
        self.topLayout.addWidget(self.input_batch_size, 0, 10)
        self.topLayout.addWidget(self.label_thread_num, 0, 11)
        self.topLayout.addWidget(self.input_thread_num, 0, 12)

        self.middleLayout.addWidget(self.tree_view)


        self.buttonLayout.addWidget(self.button_start)
        self.buttonLayout.addWidget(self.button_stop)

        pass
    

    def __adjust__(self):
        pass


    def __handle_exit__(self, signum, frame):
        import sys
        self.close()
        sys.exit(0)




if __name__ == "__main__":
    from PySide6.QtWidgets import QApplication
    import sys

    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())