from PySide6.QtCore import Signal, QObject

class SortTreeSignal(QObject):
    CreateSettingsWindow = Signal(object)


class SearchSignal(QObject):
    search_progress_info = Signal(int, int, int, str, float, float)#batch_id, total_batch, seed_num, last_seed, start_time, current_time
    """
    Args:
        batch_id: int - 当前处理的批次ID
        total_batch: int - 总批次数量
        total_seed_num: int - 当前批次找到的种子总数
        last_seed: str - 最后处理的种子编号
        start_time: float - 搜索开始时间戳
        current_time: float - 当前时间戳
    """
    searchEndNormal = Signal()
    searchEnd = Signal()
