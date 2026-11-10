from csv import reader

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QApplication, QFrame, QHBoxLayout, QFileDialog
from qfluentwidgets import BodyLabel, PushButton, VBoxLayout

from CApi import SeedManager, resource_rate_c
from config import cfg
from language import tr
from logger import log

from .worker import SearchThread
from ..Messenger import SearchMessages
from .progress import UserLayout
from .import_seed_info import ImportSeedInfo
from ..Compoents import AutoFixedConfigComboBox
from ..Compoents.Widgets.line_edit import LimitLineEdit
from .conditions.editor import SortTreeWidget


class SearchInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("searchLayout")
        self.searchLayout = VBoxLayout(self)
        self.seed_manager = SeedManager()
        self.__build__()
        self.search_thread = SearchThread(self.seed_manager, self)
        SearchMessages.searchEnd.connect(self._on_search_finish)
        self.__switch_search_mode(cfg.config.search_mode)

    def get_search_task(self) -> SearchThread:
        """返回退出协调及运行状态检查所需的搜索任务。"""
        return self.search_thread

    def __init__layout__(self):
        self.topLayout = QHBoxLayout()
        self.topLayout.setAlignment(Qt.AlignmentFlag.AlignLeft)
        self.searchLayout.addLayout(self.topLayout)
        self.middleLayout = QHBoxLayout()
        self.searchLayout.addLayout(self.middleLayout, stretch=1)
        self.userLayout = UserLayout()
        self.userLayout.search_mode_switch.checkedChanged.connect(self._on_search_mode_changed)
        self.searchLayout.addLayout(self.userLayout)
        self.buttonLayout = QHBoxLayout()
        self.searchLayout.addStretch()
        self.searchLayout.addLayout(self.buttonLayout)
        self.searchLayout.setContentsMargins(10, 10, 10, 10)

    def __init__widgets__(self):
        self.add_file_button = PushButton(tr("search.main.import_seed"))
        self.add_file_button.clicked.connect(self.__on_add_file_button_clicked)
        self.del_file_button = PushButton(tr("search.main.clear_seed"))
        self.del_file_button.clicked.connect(self.__on_del_file_button_clicked)
        self.seed_info_label = ImportSeedInfo(self.seed_manager)
        self.label_seed_range = BodyLabel(tr("search.main.seed_range"))
        self.input_seed_start = LimitLineEdit("start_seed", min_value=0, max_value=99999999, default_value=0, empty_invisible=False)
        self.input_seed_start.setMaximumWidth(125)
        # self.input_seed_start.setMinimumWidth(80)
        self.input_seed_end = LimitLineEdit("end_seed", min_value=0, max_value=99999999, default_value=99999, empty_invisible=False)
        self.input_seed_end.setMaximumWidth(125)
        # self.input_seed_end.setMinimumWidth(80)
        self.label_star_num = BodyLabel(tr("search.main.star_count"))
        self.input_star_num_start = LimitLineEdit("start_star_num", min_value=32, max_value=64, default_value=32, empty_invisible=False)
        self.input_star_num_start.setMaximumWidth(125)
        self.input_star_num_end = LimitLineEdit("end_star_num", min_value=32, max_value=64, default_value=64, empty_invisible=False)
        self.input_star_num_end.setMaximumWidth(125)
        self.label_resource_rate = BodyLabel(tr("search.main.resource_rate"))
        self.input_resource_rate =  AutoFixedConfigComboBox("resource_rate", None, cfg.config, "str", resource_rate_c, domain="resource_rates")
        # self.label_batch_size = BodyLabel("批处理大小:")
        # self.input_batch_size = LimitLineEdit("batch_size", min_value=1, max_value=4096, default_value=64, empty_invisible=False)
        # self.label_thread_num = BodyLabel("进程数:")
        # self.input_thread_num = LimitLineEdit("max_thread", min_value=1, max_value=128, default_value=cpu_count(), empty_invisible=False)
        self.button_start = PushButton(tr("search.main.start"))
        self.button_start.clicked.connect(self.__on_button_start_clicked)

        self.button_wait = PushButton(tr("search.main.pause"))
        self.button_wait.clicked.connect(self.__on_button_wait_clicked)
        self.button_wait.setEnabled(False)

        self.button_stop = PushButton(tr("search.main.stop"))
        self.button_stop.clicked.connect(self.__on_button_stop_clicked)
        self.button_stop.setEnabled(False)

        self.tree_view = SortTreeWidget()

    def __build__(self):
        self.__init__layout__()
        self.__init__widgets__()

        self.topLayout.addWidget(self.add_file_button)
        self.topLayout.addWidget(self.seed_info_label)
        self.topLayout.addWidget(self.del_file_button)

        self.topLayout.addWidget(self.label_seed_range)
        self.topLayout.addWidget(self.input_seed_start)
        self.seed_to_label = BodyLabel(tr("search.main.to"))
        self.topLayout.addWidget(self.seed_to_label)
        self.topLayout.addWidget(self.input_seed_end)
        # self.topLayout.addWidget(self.seed_step_range, 0, 4)
        self.topLayout.addWidget(self.label_star_num)
        self.topLayout.addWidget(self.input_star_num_start)
        self.star_to_label = BodyLabel(tr("search.main.to"))
        self.topLayout.addWidget(self.star_to_label)
        self.topLayout.addWidget(self.input_star_num_end)
        self.topLayout.addWidget(self.label_resource_rate)
        self.topLayout.addWidget(self.input_resource_rate)
        # self.topLayout.addWidget(self.label_batch_size)
        # self.topLayout.addWidget(self.input_batch_size)
        # self.topLayout.addWidget(self.label_thread_num)
        # self.topLayout.addWidget(self.input_thread_num)
        self.topLayout.addStretch()
        self.middleLayout.addWidget(self.tree_view)

        self.buttonLayout.addWidget(self.button_start)
        self.buttonLayout.addWidget(self.button_wait)
        self.buttonLayout.addWidget(self.button_stop)

        self.__recover_condition()

    def __recover_condition(self):
        galaxy_condition = cfg.config.galaxy_condition
        galaxy_leaf = self.tree_view.tree.addLeaf(galaxy_condition)
        for star_condition in galaxy_condition.star_condition:
            star_leaf = galaxy_leaf.addStarLeaf(star_condition)
            for planet_condition in star_condition.planet_condition:
                planet_leaf = star_leaf.addPlanetLeaf(planet_condition)
                for moon_condition in planet_condition.moon_conditions:
                    planet_leaf.addMoonLeaf(moon_condition)
        for planet_condition in galaxy_condition.planet_condition:
            planet_leaf = galaxy_leaf.addPlanetLeaf(planet_condition)
            for moon_condition in planet_condition.moon_conditions:
                planet_leaf.addMoonLeaf(moon_condition)

    def _on_search_mode_changed(self, checked: bool):
        self.__switch_search_mode(0 if not checked else 1)

    def __switch_search_mode(self, mode: int):
        if mode == 0:
            self.label_seed_range.setHidden(False)
            self.input_seed_start.setHidden(False)
            self.input_seed_end.setHidden(False)
            self.label_star_num.setHidden(False)
            self.input_star_num_start.setHidden(False)
            self.input_star_num_end.setHidden(False)
            self.seed_to_label.setHidden(False)
            self.star_to_label.setHidden(False)

            self.add_file_button.setHidden(True)
            self.del_file_button.setHidden(True)
            self.seed_info_label.setHidden(True)
        else:
            self.label_seed_range.setHidden(True)
            self.input_seed_start.setHidden(True)
            self.input_seed_end.setHidden(True)
            self.label_star_num.setHidden(True)
            self.input_star_num_start.setHidden(True)
            self.input_star_num_end.setHidden(True)
            self.seed_to_label.setHidden(True)
            self.star_to_label.setHidden(True)

            self.add_file_button.setHidden(False)
            self.del_file_button.setHidden(False)
            self.seed_info_label.setHidden(False)

    def __on_add_file_button_clicked(self):
        if self.search_thread.isRunning():
            return
        file_paths, _ = QFileDialog.getOpenFileNames(
            self.window(),
            tr("search.main.select_csv"),
            '',
            'CSV Files (*.csv);'
        )
        for file_path in file_paths:
            with open(file_path, "r", encoding="utf-8") as f:
                data = reader(f)
                for i, row in enumerate(data):
                    try:
                        seed = int(row[0])
                        star_num = int(row[1])
                    except Exception:
                        continue

                    if not (0 <= seed <= 99999999 and 32 <= star_num <= 64):
                        continue

                    self.seed_manager.add_seed(seed, star_num)

                    if i % 1000 == 0:
                        QApplication.processEvents()
        self.seed_info_label.fresh()

    def __on_del_file_button_clicked(self):
        if self.search_thread.isRunning():
            return
        self.seed_manager.clear()
        self.seed_info_label.fresh()

    def __on_button_start_clicked(self):
        if self.search_thread.isRunning():
            self.search_thread.end_wait()
            self.button_start.setEnabled(False)
            self.button_wait.setEnabled(True)
            return
        if cfg.config.search_mode == 1 and self.seed_manager.get_seeds_count() == 0:
            log.info("当前无可搜索种子，请先导入种子文件或修改搜索范围")
            return

        seeds = (cfg.config.start_seed, cfg.config.end_seed)
        star_nums = (cfg.config.start_star_num, cfg.config.end_star_num)
        # self.userLayout.seeds = seeds

        log.info("开始搜索")
        self.button_start.setEnabled(False)
        if cfg.config.search_mode == 0:
            task_num = (seeds[1]-seeds[0]+1)*(star_nums[1]-star_nums[0]+1)
        else:
            task_num = self.seed_manager.get_seeds_count()
        if task_num > 1000000000:
            self.userLayout.progressBar.setMaximum(task_num // 10)
        else:
            self.userLayout.progressBar.setMaximum(task_num)
        self.userLayout.seedInfoLabel.setText("")
        # log.debug(self.userLayout.progressBar.maximum())
        self.search_thread.end_flag = False
        self.search_thread.wait_flag = False
        self.search_thread.start()
        self.button_stop.setEnabled(True)
        self.button_wait.setEnabled(True)

    def __on_button_wait_clicked(self):
        self.search_thread.start_wait()
        self.button_wait.setEnabled(False)
        self.button_start.setEnabled(True)

    def __on_button_stop_clicked(self):
        self.search_thread.terminate()

    def _on_search_finish(self):
        self.button_start.setEnabled(True)
        self.button_stop.setEnabled(False)
        self.button_wait.setEnabled(False)

