from GUI.MainWindow import MainWindow
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import Qt
import sys
import os
from config import cfg
import multiprocessing
from CApi import *

def main():
    do_init_c()
    set_local_size_c(cfg.config.local_size)
    if not set_device_id_c(cfg.config.device_id):
        print("Set device id failed! Roll back to cpu!")
        set_device_id_c(-1)
    cfg.config.local_size = get_local_size_c()
    cfg.config.device_id = get_device_id_c()
    scale_factor = cfg.config.ui_scale_factor
    if scale_factor != 1.0:
        os.environ["QT_SCALE_FACTOR"] = str(scale_factor)
        # os.environ["QT_ENABLE_HIGHDPI_SCALING"] = "1"

    app = QApplication(sys.argv)

    app.setAttribute(Qt.AA_DontCreateNativeWidgetSiblings)

    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    multiprocessing.freeze_support()
    os.chdir(
        os.path.dirname(sys.executable)
        if getattr(sys, "frozen", False)
        else os.path.dirname(os.path.abspath(__file__))
    )
    main()
