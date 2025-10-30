from typing import Any
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QWidget,
    QTreeWidgetItem,
    QFileSystemModel,
    QHBoxLayout,
    QComboBox,
    QSizePolicy,
    QHeaderView,
    QGridLayout,
)
from qfluentwidgets import TreeWidget, CaptionLabel
from CApi import GalaxyData, StarData, PlanetData
from GUI import vein_names

class AstroTree(TreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.leaf = None
        self.setHeaderHidden(False)
        self.setEditTriggers(TreeWidget.NoEditTriggers)  # 先禁用自动编辑
        self.setUniformRowHeights(True)
        self.setColumnCount(2)

        self.setHeaderLabels(["类型", "信息"])

        header = self.header()
        # header.setStretchLastSection(False)  # 最后一列拉伸
        header.setSectionResizeMode(0, QHeaderView.Interactive)
        header.setSectionResizeMode(1, QHeaderView.Stretch)
        # header.setSectionResizeMode(2, QHeaderView.Fixed)

        self.setColumnWidth(0, 170)
        # self.setColumnWidth(1, 550)

        self.root = self.invisibleRootItem()

        self.root.setExpanded(True)

        self.setContextMenuPolicy(Qt.NoContextMenu)

    def fresh(self, galaxy_data: GalaxyData|None = None) -> None:
        if self.leaf is not None:
            self.root.removeChild(self.leaf)
        if galaxy_data is not None:
            self.leaf = GalaxyTreeWidgetItem(self, galaxy_data, self.root)

class GalaxyTreeWidgetItem(QTreeWidgetItem):
    def __init__(self, root: "AstroTree", galaxy_data: GalaxyData, parent = None):
        if parent is None:
            super().__init__()
        else:
            super().__init__(parent)
        self.root = root
        self.galaxy_data = galaxy_data

        show_text = []
        show_text.append(str(galaxy_data.seed))
        show_text.append(str(galaxy_data.star_num))

        self.setText(0, "星系")
        self.setText(1, "|".join(show_text))

        for star_data in galaxy_data.stars:
            self.addChild(StarTreeWidgetItem(self.root, star_data, self))

        self.setExpanded(True)

class StarTreeWidgetItem(QTreeWidgetItem):
    def __init__(self, root: "AstroTree", star_data: StarData, parent = None):
        if parent is None:
            super().__init__()
        else:
            super().__init__(parent)
        self.root = root
        self.star_data = star_data

        show_text = []
        show_text.append(f"{star_data.dyson_lumino:.2f}L")
        show_text.append(f"{star_data.distance:.1f}LY")

        for i in range(6, 14):
            if star_data.veins[i] > 0:
                show_text.append(vein_names[i])

        self.setText(0, star_data.type)
        self.setText(1, "|".join(show_text))

        for planet_data in star_data.planets:
            self.addChild(PlanetTreeWidgetItem(self.root, planet_data, self))

        self.setExpanded(True)

class PlanetTreeWidgetItem(QTreeWidgetItem):
    def __init__(self, root: "AstroTree", planet_data: PlanetData, parent = None):
        if parent is None:
            super().__init__()
        else:
            super().__init__(parent)
        self.root = root
        self.planet_data = planet_data

        show_text = []
        if planet_data.is_in_dsp:
            show_text.append("全包")
        elif planet_data.is_on_dsp:
            show_text.append("全接收")
        
        for i in planet_data.singularity:
            show_text.append(i)

        if planet_data.liquid == 1:
            show_text.append("水")
        elif planet_data.liquid == 2:
            show_text.append("硫酸")

        for i in range(6, 14):
            if planet_data.veins[i] > 0:
                show_text.append(vein_names[i])

        self.setText(0, planet_data.type)
        self.setText(1, "|".join(show_text))

        self.setExpanded(True)
