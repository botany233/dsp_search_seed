from PySide6.QtCore import Signal, QObject

class SortTreeSignal(QObject):
    CreateSettingsWindow = Signal(object)


class SearchSignal(QObject):
    search_progress = Signal(int)
    search_last_seed = Signal(str)
    searchEndNormal = Signal()
    searchEnd = Signal()
