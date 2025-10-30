from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QGridLayout, QTreeWidgetItem
from PySide6.QtCore import Qt
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, CaptionLabel
from GUI import vein_names
from .astro_tree import GalaxyTreeWidgetItem, StarTreeWidgetItem, PlanetTreeWidgetItem

class AstroInfo(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.main_layout = QVBoxLayout(self)
        self.veins_label = CaptionLabel()
        self.main_layout.addWidget(self.veins_label)

        self.other_label = CaptionLabel()
        self.main_layout.addWidget(self.other_label)

    def fresh(self, item: QTreeWidgetItem) -> None:
        if isinstance(item, GalaxyTreeWidgetItem):
            galaxy_data = item.galaxy_data
            self.veins_label.setText(get_veins_text(galaxy_data.veins, galaxy_data.gas_veins, galaxy_data.liquid))
            self.other_label.setText("星系其它信息")
        elif isinstance(item, StarTreeWidgetItem):
            star_data = item.star_data
            self.veins_label.setText(get_veins_text(star_data.veins, star_data.gas_veins, star_data.liquid))
            self.other_label.setText("恒星其它信息")
        elif isinstance(item, PlanetTreeWidgetItem):
            planet_data = item.planet_data
            self.veins_label.setText(get_veins_text(planet_data.veins, planet_data.gas_veins, planet_data.liquid))
            self.other_label.setText("行星其它信息")

def get_veins_text(veins: list[int], gas_veins: list[float], liquid: list[int]|int) -> str:
    text = []
    for i in range(6):
        text.append(f"{vein_names[i]}: {veins[i]}")

    for i in range(6, 14):
        if veins[i] > 0:
            text.append(f"{vein_names[i]}: {veins[i]}")

    if isinstance(liquid, int):
        if liquid == 1:
            text.append("水：海洋")
        elif liquid == 2:
            text.append("硫酸：海洋")
    else:
        if liquid[1] > 0:
            text.append("水：海洋")
        elif liquid[2] > 0:
            text.append("硫酸：海洋")

    if gas_veins[0] > 0:
        text.append(f"氢: {gas_veins[0]:.2f}/s")
    if gas_veins[1] > 0:
        text.append(f"重氢: {gas_veins[1]:.2f}/s")
    if gas_veins[2] > 0:
        text.append(f"可燃冰: {gas_veins[2]:.2f}/s")
    return "\n".join(text)
