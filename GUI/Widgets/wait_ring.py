from qfluentwidgets import IndeterminateProgressRing, MaskDialogBase, BodyLabel
from PySide6.QtWidgets import QVBoxLayout, QGridLayout, QWidget
from PySide6.QtCore import Qt

class WaitRing(MaskDialogBase):
    """ A wait ring dialog """

    def __init__(self, parent):
        super().__init__(parent)

        self.vLayout = QVBoxLayout()

        self.overlayWidget = QWidget()
        self.overlayLayout = QGridLayout(self.overlayWidget)
        self.overlayLayout.setContentsMargins(0, 0, 0, 0)
        self.ring = IndeterminateProgressRing()
        self.ring.setFixedSize(100, 100)
        self.textLabel = BodyLabel("结束进程中")
        # 将进度环和文字放在同一个位置，文字在上层
        self.overlayLayout.addWidget(self.ring, 0, 0, Qt.AlignCenter)
        self.overlayLayout.addWidget(self.textLabel, 0, 0, Qt.AlignCenter)
        
        self.vLayout.addWidget(self.overlayWidget, 0, Qt.AlignCenter)
        self._hBoxLayout.addLayout(self.vLayout, 1)

    def start(self):
        self.ring.start()
        self.show()

    def stop(self):
        self.ring.stop()
        self.hide()

if __name__ == "__main__":
    import sys
    from PySide6.QtWidgets import QApplication, QVBoxLayout, QWidget
    app = QApplication(sys.argv)

    main_window = QWidget()
    main_window.setFixedSize(500,500)
    main_layout = QVBoxLayout(main_window)

    wait_ring = WaitRing(main_window)


    main_window.show()
    sys.exit(app.exec())