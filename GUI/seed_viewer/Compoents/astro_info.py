from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QGridLayout, QTreeWidgetItem
from PySide6.QtCore import Qt
from PySide6.QtGui import QPixmap
from qfluentwidgets import TitleLabel, BodyLabel, CaptionLabel, getFont, isDarkTheme
from .astro_tree import GalaxyTreeWidgetItem, StarTreeWidgetItem, PlanetTreeWidgetItem
from CApi import GalaxyData, StarData, PlanetData, vein_names_c, star_types_c

from GUI.Compoents.Widgets import GlowLabelBase
from GUI.dsp_icons import DSPIcons
from GUI import singularity

class GlowBodyLabel(GlowLabelBase, BodyLabel):
    def _init(self):
        super()._init()
        self.setup_glow_effect()
        return self

class InfoLabel(CaptionLabel):
    def getFont(self):
        return getFont(13)

class GlowCaptionLabel(GlowLabelBase, InfoLabel):
    def _init(self):
        super()._init()
        self.setup_glow_effect()
        return self

GLOW_LIST = set(
    [
        "硫酸",
        "油",
        "可燃冰",
        "刺笋结晶",
        "分型硅",
        "单极磁石",
        "光栅石",
        "金伯利",
        "有机晶体",
        "全包星",
        "全接收星",
        "潮汐锁定永昼永夜",
    ]
)
GLOW_LIST.update(singularity)

class AstroInfo(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.main_layout = QVBoxLayout(self)
        self.main_widget = None
        lt_qss = """AstroInfo{border: 1px solid #ededed; border-radius: 8px;}"""
        dk_qss = """AstroInfo{border: 1px solid #2B2B2B; border-radius: 8px;}"""
        if isDarkTheme():
            self.setStyleSheet(dk_qss)
        else:
            self.setStyleSheet(lt_qss)

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

class InfoBase(QWidget):
    def __init__(self, data: object, parent=None):
        super().__init__(parent)
        self.data = data

        self.veins_count = 0

        self.main_layout = QVBoxLayout(self)
        self.main_layout.setContentsMargins(0,0,0,0)
        self.title_label = TitleLabel()
        self.sub_title_layout = QHBoxLayout()
        self.main_layout.addWidget(self.title_label)
        self.main_layout.addLayout(self.sub_title_layout)
        self.sub_title_label = BodyLabel("")
        self.sub_title_glow_label = GlowBodyLabel("")
        self.sub_title_layout.setAlignment(Qt.AlignLeft)
        self.sub_title_layout.addWidget(self.sub_title_label)
        self.sub_title_layout.addWidget(self.sub_title_glow_label)

        self.info_layout = QHBoxLayout()

        self.veins_V_layout = QVBoxLayout()
        self.info_layout.addLayout(self.veins_V_layout)

        self.veins_layout = QGridLayout()

        self.veins_layout.setVerticalSpacing(0)

        # 在veins_layout上方添加分界线
        self.top_separator = QFrame()
        self.top_separator.setFrameShape(QFrame.HLine)
        self.top_separator.setFrameShadow(QFrame.Sunken)
        self.top_separator.setLineWidth(100)
        self.top_separator.setMidLineWidth(0)

        # 在veins_layout下方添加分界线
        self.bottom_separator = QFrame()
        self.bottom_separator.setFrameShape(QFrame.HLine)
        self.bottom_separator.setFrameShadow(QFrame.Sunken)
        self.bottom_separator.setLineWidth(100)
        self.bottom_separator.setMidLineWidth(0)

        self.veins_V_layout.addWidget(self.top_separator)

        self.veins_V_layout.addLayout(self.veins_layout)

        self.veins_V_layout.addWidget(self.bottom_separator)

        self.main_layout.addLayout(self.info_layout)

    def add_subtitle(self, title: list[str] | str) -> None:
        old_sub_title = self.sub_title_label.text()
        old_glow_title = self.sub_title_glow_label.text()
        if isinstance(title, str):
            if title in GLOW_LIST:
                self.sub_title_glow_label.setText(old_glow_title + title)
            else:
                self.sub_title_label.setText(old_sub_title + title)
        else:
            for text in title:
                if text in GLOW_LIST:
                    if old_glow_title != "":
                        old_glow_title += " "
                    self.sub_title_glow_label.setText(old_glow_title + text)
                    old_glow_title = self.sub_title_glow_label.text()
                else:
                    if old_sub_title != "":
                        old_sub_title += " "
                    self.sub_title_label.setText(old_sub_title + text)
                    old_sub_title = self.sub_title_label.text()

    def add_veins(self, veins: list[str]) -> None:
        for vein in veins:
            name, value = vein.split("：")
            if name in GLOW_LIST:
                vein_label = GlowCaptionLabel(name)
            else:
                vein_label = InfoLabel(name)
            vein_value_label = InfoLabel(value)
            icon = InfoLabel("")
            icon.setPixmap(QPixmap(DSPIcons.match_icon(name).path()).scaled(17,17))
            self.veins_layout.addWidget(vein_label, self.veins_count, 0)
            self.veins_layout.addWidget(icon, self.veins_count, 1)
            self.veins_layout.addWidget(vein_value_label, self.veins_count, 2)
            self.veins_layout.setAlignment(vein_value_label, Qt.AlignRight)
            self.veins_layout.setAlignment(icon, Qt.AlignCenter)
            self.veins_count += 1

    def add_stars(self, stars: list[str]) -> None:
        """应该在`add_veins`之后调用"""
        count = len(stars)
        df_count = (self.veins_count - count) // 2
        for i in range(count):
            name, value = stars[i].split("：")
            star_name_label = InfoLabel(name)
            star_value_label = InfoLabel(value)
            self.veins_layout.addWidget(star_name_label, i + df_count, 3)
            self.veins_layout.addWidget(star_value_label, i + df_count, 4)
            self.veins_layout.setAlignment(star_value_label, Qt.AlignRight)


class GalaxyInfo(InfoBase):
    def __init__(self, data: GalaxyData, parent=None):
        super().__init__(data, parent)
        self.bottom_separator.hide()
        self.title_label.setText("星系信息")
        self.sub_title_label.setText(f"{data.star_num}星")

        self.add_veins(get_veins_list(data.veins_point, data.gas_veins, data.liquid))

        star_type_nums = [0] * 14
        for star in data.stars:
            star_type_nums[star.type_id-1] += 1
        star_labels = [f"{star_types_c[i]}：{star_type_nums[i]}" for i in range(14)]
        self.add_stars(star_labels)


class StarInfo(InfoBase):
    def __init__(self, data: StarData, parent=None):
        super().__init__(data, parent)

        self.title_label.setText(data.name)

        self.sub_title_label.setText(data.type)


        self.add_veins(get_veins_list(data.veins_point, data.gas_veins, data.liquid))


        other_label = CaptionLabel(f'''\
戴森球半径：{data.dyson_radius*40000:.0f}m
戴森球光度：{data.dyson_lumino:.2f}L
距离：{data.distance:.2f}LY''')
        self.main_layout.addWidget(other_label)

class PlanetInfo(InfoBase):
    def __init__(self, data: PlanetData, parent=None):
        super().__init__(data, parent)

        self.title_label.setText(data.name)

        sub_title_text = [data.type]
        if data.dsp_level == 2:
            sub_title_text.append("全包星")
        elif data.dsp_level == 1:
            sub_title_text.append("全接收星")
        sub_title_text.extend(data.singularity_str)
        self.add_subtitle(sub_title_text)

        self.add_veins(get_veins_list(data.veins_point, data.gas_veins, data.liquid, data.is_gas))
        other_label = CaptionLabel(f'''\
风能利用率：{data.wind*100:.0f}%
光能利用率：{data.lumino*100:.0f}%''')
        self.main_layout.addWidget(other_label)

def get_veins_list(veins: list[int], gas_veins: list[float], liquid: list[int]|int, is_gas = False) -> list[str]:
    text = []

    if not is_gas:
        for i in range(6):
            text.append(f"{vein_names_c[i]}：{veins[i]}")

        for i in range(6, 14):
            if veins[i] > 0:
                text.append(f"{vein_names_c[i]}：{veins[i]}")

        if isinstance(liquid, int):
            if liquid == 1:
                text.append("水：海洋")
            elif liquid == 2:
                text.append("硫酸：海洋")
        else:
            if liquid[1] > 0:
                text.append("水：海洋")
            if liquid[2] > 0:
                text.append("硫酸：海洋")

    if gas_veins[0] > 0:
        text.append(f"氢：{gas_veins[0]:.2f}/s")
    if gas_veins[1] > 0:
        text.append(f"重氢：{gas_veins[1]:.2f}/s")
    if gas_veins[2] > 0:
        text.append(f"可燃冰：{gas_veins[2]:.2f}/s")
    return text
