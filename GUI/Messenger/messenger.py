from PySide6.QtCore import Signal, QObject

class SortTreeSignal(QObject):
    CreateSettingsWindow = Signal(object)


class SearchSignal(QObject):
    search_progress_info = Signal(int, int, int, str, float, float)#batch_id, total_batch, seed_num, last_seed, start_time, current_time
    searchEndNormal = Signal()
    searchEnd = Signal()
