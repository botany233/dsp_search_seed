from typing import Any
from PySide6.QtCore import QEvent, QObject, Qt
from PySide6.QtGui import QMouseEvent, QResizeEvent
from PySide6.QtWidgets import (
    QWidget,
    QTreeWidgetItem,
    QFileSystemModel,
    QHBoxLayout,
    QComboBox,
    QSizePolicy,
    QHeaderView,
    QGridLayout,
    QTreeWidget,
)
from qfluentwidgets import TreeWidget, CaptionLabel, BodyLabel
from CApi import GalaxyData, StarData, PlanetData, resource_rate_c
from GUI import vein_names, singularity
from language import tr, tr_any_domain, tr_domain

from GUI.Widgets import WaitRing

COLORFUL_TEXTS = set(
    [
        "全包",
        "全接收",
    ]
)

COLORFUL_TEXTS.update(singularity)

class IgnoreLabel(CaptionLabel):

    def _init(self):
        super()._init()
        self.setTextInteractionFlags(Qt.NoTextInteraction)
        self.setFocusPolicy(Qt.NoFocus)
        return self
    
    def eventFilter(self, watched: QObject, event: QEvent) -> bool:
        event.ignore()
        return True
    def mouseDoubleClickEvent(self, event: QMouseEvent) -> None:
        event.ignore()
        return None
    def mousePressEvent(self, event: QMouseEvent) -> None:
        event.ignore()
        return None
    def mouseMoveEvent(self, ev: QMouseEvent) -> None:
        ev.ignore()
        return None
    
    def set_texts(self, texts: list[str] | str) -> None:
        if isinstance(texts, str):
            display_text = tr_any_domain(texts)
            if texts in COLORFUL_TEXTS:
                display_text = f"<font color=#409EFF>{display_text}</font>"
            self.setText(display_text)
            return
        else:
            display_texts = []
            for text in texts:
                display_text = tr_any_domain(text)
                if text in COLORFUL_TEXTS:
                    display_text = f"<font color=#409EFF>{display_text}</font>"
                display_texts.append(display_text)
            self.setText("|".join(display_texts))


class AstroTree(TreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.leaf = None
        self.setHeaderHidden(False)
        self.setEditTriggers(TreeWidget.NoEditTriggers)  # 先禁用自动编辑
        self.setUniformRowHeights(True)
        self.setColumnCount(2)
        self.setBorderRadius(8)
        self.setBorderVisible(True)

        self.setHeaderLabels([tr("viewer.astro_tree.type"), tr("viewer.astro_tree.info")])

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

        self.wait_ring = WaitRing(self)
        self.wait_ring.textLabel.setText(tr("viewer.astro_tree.loading"))
        self.wait_ring.stop()

    def resizeEvent(self, event: QResizeEvent) -> None:
        ret = super().resizeEvent(event)
        self.wait_ring.resize(self.size())
        return ret

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
        show_text.append(str(galaxy_data.seed_id))
        show_text.append(str(galaxy_data.star_num))
        show_text.append(resource_rate_c[galaxy_data.resource_index])

        self.setText(0, tr("viewer.astro_tree.galaxy"))
        
        info_label = IgnoreLabel()
        info_label.set_texts(show_text)
        self.root.setItemWidget(self, 1, info_label)

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
        show_text.append(f"{star_data.distance:.1f}LY")

        for i in range(6, 14):
            if star_data.veins_point[i] > 0:
                show_text.append(vein_names[i])

        self.setText(0, tr_domain("star_types", star_data.type))
        info_label = IgnoreLabel()
        info_label.set_texts(show_text)
        self.root.setItemWidget(self, 1, info_label)

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
        if planet_data.dsp_level == 2:
            show_text.append("全包")
        elif planet_data.dsp_level == 1:
            show_text.append("全接收")

        for i in planet_data.singularity_str:
            show_text.append(i)

        if planet_data.liquid == 1:
            show_text.append("水")
        elif planet_data.liquid == 2:
            show_text.append("硫酸")

        for i in range(6, 14):
            if planet_data.veins_point[i] > 0:
                show_text.append(vein_names[i])

        self.setText(0, tr_domain("planet_types", planet_data.type))
        info_label = IgnoreLabel()
        info_label.set_texts(show_text)
        self.root.setItemWidget(self, 1, info_label)

        self.setExpanded(True)
