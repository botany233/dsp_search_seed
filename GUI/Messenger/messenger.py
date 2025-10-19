from PySide6.QtCore import Signal, QObject


class SortTreeSignal(QObject):
    
    CreateSettingsWindow = Signal(object)