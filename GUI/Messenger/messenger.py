from PySide6.QtCore import Signal, QObject

class SortTreeSignal(QObject):
    CreateSettingsWindow = Signal(object)


class SearchSignal(QObject):
    search_progress_info = Signal(int, int, float)#batch_id, total_batch, use_time
    """
    Args:
        batch_id: int - 当前处理的批次ID
        total_batch: int - 总批次数量
        total_seed_num: int - 当前批次找到的种子总数
        last_seed: str - 最后处理的种子编号
        start_time: float - 搜索开始时间戳
        current_time: float - 当前时间戳
    """
    searchEnd = Signal()
    new_find_seed = Signal(int, int, int)#total_seed_num, last_seed_id, last_star_num
