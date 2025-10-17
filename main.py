from GUI.MainWindow import MainWindow
from PySide6.QtWidgets import QApplication
import sys


def main():
    


    app = QApplication()
    window = MainWindow()
    window.show()
    sys.exit(app.exec())



if __name__ == "__main__":
    main()
