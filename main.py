from GUI.MainWindow import MainWindow
from PySide6.QtWidgets import QApplication
import sys
import os
from config import cfg


def main():
    scale_factor = cfg.config.ui_scale_factor
    if scale_factor != 1.0:
        os.environ["QT_SCALE_FACTOR"] = str(scale_factor)
        # os.environ["QT_ENABLE_HIGHDPI_SCALING"] = "1"
    app = QApplication()

    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    os.chdir(
        os.path.dirname(sys.executable)
        if getattr(sys, "frozen", False)
        else os.path.dirname(os.path.abspath(__file__))
    )
    main()
