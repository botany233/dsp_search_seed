__all__ = ["AlwaysShowLabel", "GlowLabelBase"]
from qfluentwidgets import FluentLabelBase, BodyLabel, getFont, CaptionLabel, isDarkTheme
from PySide6.QtCore import Signal, QTimer
from PySide6.QtWidgets import QGraphicsDropShadowEffect
from PySide6.QtGui import QColor, QFont, QPainter, QPen


class AlwaysShowLabel(BodyLabel):
    onTextChanged = Signal()

    def _init(self):
        self.onTextChanged.connect(self._resize)
        QTimer.singleShot(0, self._resize)
        return super()._init()

    def setText(self, arg__1: str) -> None:
        self.onTextChanged.emit()
        return super().setText(arg__1)

    def _resize(self):
        length = self.fontMetrics().horizontalAdvance(self.text()) + 10
        self.setMinimumWidth(length)
        width = self.fontMetrics().horizontalAdvance(self.text()) + 10
        self.setFixedWidth(width)


class GlowLabelBase():
    outline_alpha = 100
    def setup_glow_effect(self):
        # 创建阴影效果
        shadow_effect = QGraphicsDropShadowEffect(self)

        shadow_effect.setBlurRadius(20)  # 辉光半径
        shadow_effect.setColor(QColor(255,231,95))  # 辉光颜色
        shadow_effect.setOffset(0, 0)  # 偏移量为0，实现均匀辉光
        
        self.setGraphicsEffect(shadow_effect)
        self.setStyleSheet("""
            QLabel{
                color: #F5D00A;
                background: transparent;
            }
        """) # raw #F5D00A


    def setOutlineAlpha(self, alpha: int):
        self.outline_alpha = alpha
        self.update()

    def paintEvent(self, event, /):
        if isDarkTheme():
            return super().paintEvent(event)

        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # 添加字体描边效果

        painter.setPen(QColor(0, 0, 0, self.outline_alpha)) # 半透明黑色描边
        offsets = [(-1, -1), (-1, 0), (-1, 1),
                (0, -1),         (0, 1),
                (1, -1),  (1, 0), (1, 1)]

        for dx, dy in offsets:
            rect = self.rect().translated(dx, dy)
            painter.drawText(rect, self.alignment(), self.text())

        super().paintEvent(event)


class GlowLabel(CaptionLabel, GlowLabelBase):
    """后继承`GlowLabelBase` 有辉光无描边"""
    
    def _init(self):
        super()._init()
        self.setup_glow_effect()
        return self
    

class GlowRevLabel(GlowLabelBase, CaptionLabel):
    """前继承`GlowLabelBase` 有描边"""
    def _init(self):
        super()._init()
        self.setup_glow_effect()
        return self

if __name__ == "__main__":
    from PySide6.QtWidgets import QApplication, QVBoxLayout, QWidget, QFrame, QHBoxLayout
    import sys

    app = QApplication(sys.argv)

    main_window = QWidget()
    layout = QVBoxLayout(main_window)

    white = QHBoxLayout()
    layout.addLayout(white)

    glow_label = GlowLabel("This is a Glow Label\n这是一个辉光标签")
    white.addWidget(glow_label)

    glow_rev_label = GlowRevLabel("This is a Glow Label\n这是一个辉光标签")
    white.addWidget(glow_rev_label)

    qframe = QFrame()
    qframe.setStyleSheet("background-color: black;")
    qframe_layout = QHBoxLayout(qframe)
    qframe_layout.setContentsMargins(0, 0, 0, 0)

    glow_label2 = GlowLabel("This is a Glow Label \nwith black background\n这是黑色背景下的辉光标签")
    qframe_layout.addWidget(glow_label2)
    glow_rev_label2 = GlowRevLabel("This is a Glow Label \nwith black background\n这是黑色背景下的辉光标签")
    qframe_layout.addWidget(glow_rev_label2)

    layout.addWidget(qframe)

    main_window.show()
    sys.exit(app.exec())