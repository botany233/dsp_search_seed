from PySide6.QtCore import Signal, QObject

class SortTreeSignal(QObject):
    CreateSettingsWindow = Signal(object)


class SearchSignal(QObject):
    searchFinish = Signal(int)
    searchFinishOnce = Signal()
    searchEnd = Signal()


