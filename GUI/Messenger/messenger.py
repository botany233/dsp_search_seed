from PySide6.QtCore import Signal, QObject

class SortTreeSignal(QObject):
    CreateSettingsWindow = Signal(object)


class SearchSignal(QObject):
    search_progress_info = Signal(int, int, float)#batch_id, total_batch, use_time
    """
    Args:
        batch_id: int - 当前处理的批次ID
        total_batch: int - 总批次数量
        use_time: float - 搜索时间
    """
    searchEnd = Signal()
    new_find_seed = Signal(int, int, int)#total_seed_num, last_seed_id, last_star_num

class GPUBenchmarkSignal(QObject):
    result = Signal(int, float)#gpu_thread_num, speed
    end = Signal()
