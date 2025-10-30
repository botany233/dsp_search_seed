from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QGridLayout, QTreeWidgetItem
from PySide6.QtCore import Qt
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, CaptionLabel
from .astro_tree import GalaxyTreeWidgetItem, StarTreeWidgetItem, PlanetTreeWidgetItem
from CApi import GalaxyData, StarData, PlanetData, vein_names_c, star_types_c

class AstroInfo(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.main_layout = QVBoxLayout(self)
        self.main_widget = None

    def fresh(self, item: QTreeWidgetItem|None) -> None:
        if self.main_widget is not None:
            self.main_layout.removeWidget(self.main_widget)
            self.main_widget.deleteLater()
            self.main_widget = None
        if item is not None:
            if isinstance(item, GalaxyTreeWidgetItem):
                self.main_widget = GalaxyInfo(item.galaxy_data)
            elif isinstance(item, StarTreeWidgetItem):
                self.main_widget = StarInfo(item.star_data)
            elif isinstance(item, PlanetTreeWidgetItem):
                self.main_widget = PlanetInfo(item.planet_data)
            self.main_layout.addWidget(self.main_widget)

class GalaxyInfo(QWidget):
    def __init__(self, data: GalaxyData, parent=None):
        super().__init__(parent)
        self.main_layout = QVBoxLayout(self)
        main_title_label = TitleLabel("星系信息")
        self.main_layout.addWidget(main_title_label)
        sub_title_label = BodyLabel(f"{data.star_num}星")
        self.main_layout.addWidget(sub_title_label)

        text_layout = QHBoxLayout()
        veins_label = CaptionLabel(get_veins_text(data.veins, data.gas_veins, data.liquid))
        text_layout.addWidget(veins_label)
        star_label = CaptionLabel("\n".join([f"{star_types_c[i]}: {data.star_type_nums[i]}" for i in range(14)]))
        text_layout.addWidget(star_label)
        self.main_layout.addLayout(text_layout)

class StarInfo(QWidget):
    def __init__(self, data: StarData, parent=None):
        super().__init__(parent)
        self.main_layout = QVBoxLayout(self)
        main_title_label = TitleLabel(data.name)
        self.main_layout.addWidget(main_title_label)
        sub_title_label = BodyLabel(data.type)
        self.main_layout.addWidget(sub_title_label)

        veins_label = CaptionLabel(get_veins_text(data.veins, data.gas_veins, data.liquid))
        self.main_layout.addWidget(veins_label)

        other_lebel = CaptionLabel(f'''\
戴森球半径：{data.dyson_radius*40000:.0f}m
戴森球光度：{data.dyson_lumino:.2f}L
距离：{data.distance:.2f}LY''')
        self.main_layout.addWidget(other_lebel)

class PlanetInfo(QWidget):
    def __init__(self, data: PlanetData, parent=None):
        super().__init__(parent)
        self.main_layout = QVBoxLayout(self)
        main_title_label = TitleLabel(data.name)
        self.main_layout.addWidget(main_title_label)

        sub_title_text = [data.type]
        if data.is_in_dsp:
            sub_title_text.append("全包星")
        elif data.is_on_dsp:
            sub_title_text.append("全接收星")
        sub_title_text.extend(data.singularity)
        sub_title_label = BodyLabel(" ".join(sub_title_text))
        self.main_layout.addWidget(sub_title_label)

        veins_label = CaptionLabel(get_veins_text(data.veins, data.gas_veins, data.liquid, data.is_gas))
        self.main_layout.addWidget(veins_label)

        other_lebel = CaptionLabel(f'''\
风能利用率：{data.wind*100:.0f}%
光能利用率：{data.lumino*100:.0f}%''')
        self.main_layout.addWidget(other_lebel)

def get_veins_text(veins: list[int], gas_veins: list[float], liquid: list[int]|int, is_gas = False) -> str:
    text = []

    if not is_gas:
        for i in range(6):
            text.append(f"{vein_names_c[i]}: {veins[i]}")

        for i in range(6, 14):
            if veins[i] > 0:
                text.append(f"{vein_names_c[i]}: {veins[i]}")

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
