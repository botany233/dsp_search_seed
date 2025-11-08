from PySide6.QtWidgets import QFrame, QHBoxLayout, QVBoxLayout, QWidget, QFileDialog, QGridLayout, QTreeWidgetItem, QApplication
from PySide6.QtCore import Qt
from PySide6.QtWidgets import QDialog
from qfluentwidgets import TitleLabel, BodyLabel, PushButton, CaptionLabel
from datetime import datetime
from csv import reader
from .Compoents import *

class TwiceSearchInterface(QFrame):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.mainLayout = QHBoxLayout(self)