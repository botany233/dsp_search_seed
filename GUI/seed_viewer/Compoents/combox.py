from qfluentwidgets import ComboBox, setFont
from PySide6.QtWidgets import QHBoxLayout, QFrame, QGridLayout, QWidget
from PySide6.QtCore import QEvent, QRectF
from PySide6.QtGui import QPainter

vein_types = ["铁", "铜", "硅", "钛", "石", "煤", "油", "可燃冰", "金伯利",
              "分型硅", "有机晶体", "光栅石", "刺笋结晶", "单极磁石"]
planet_types = ["地中海", "气态巨星", "冰巨星", "高产气巨", "干旱荒漠", "灰烬冻土", "海洋丛林", "熔岩",
                "冰原冻土", "贫瘠荒漠", "戈壁", "火山灰", "红石", "草原", "水世界", "黑石盐滩",
                "樱林海", "飓风石林", "猩红冰湖", "热带草原", "橙晶荒漠", "极寒冻土", "潘多拉沼泽"]
star_types = ["红巨星", "黄巨星", "蓝巨星", "白巨星", "白矮星", "中子星", "黑洞",
              "A型恒星", "B型恒星", "F型恒星", "G型恒星", "K型恒星", "M型恒星", "O型恒星"]
custom_types = ["例子：种子号", "例子：恒星数", "自定义1", "自定义2", "自定义3"]

sort_types = {
    "矿物": vein_types,
    "行星类别": planet_types,
    "恒星类别": star_types,
    "自定义": custom_types
}

class MainTypeComboBox(ComboBox):
    def __init__(self):
        super().__init__()
        # self.currentIndexChanged.connect(self._on_currentIndexChanged)
        setFont(self, 12)
        texts = sort_types.keys()
        self.addItems(texts)
        max_width = max(self.fontMetrics().horizontalAdvance(text) for text in texts)
        self.setMinimumWidth(max_width + 45)

class SubTypeComboBox(ComboBox):
    def __init__(self, main_comobox: MainTypeComboBox):
        super().__init__()
        self.main_comobox = main_comobox
        setFont(self, 12)
        texts = []
        for i in sort_types.values():
            texts += i
        max_width = max(self.fontMetrics().horizontalAdvance(text) for text in texts)
        self.setMinimumWidth(max_width + 45)
        self.main_comobox.currentIndexChanged.connect(self.refresh)
        self.refresh()

    def refresh(self, *args):
        texts = sort_types[self.main_comobox.currentText()]
        self.clear()
        self.addItems(texts)
