from GUI.MainWindow import MainWindow
from PySide6.QtWidgets import QApplication
import sys
import os

def main():
    


    app = QApplication()
    window = MainWindow()
    window.show()
    sys.exit(app.exec())



if __name__ == "__main__":
    os.chdir(
    os.path.dirname(sys.executable) if getattr(sys, 'frozen', False) else os.path.dirname(os.path.abspath(__file__)))
    main()
