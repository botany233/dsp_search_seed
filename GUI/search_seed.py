from PySide6.QtCore import QThread, QMutex
from .Messenger import SearchMessages

from config import cfg
from logger import log

class SearchThread(QThread):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.mutex = QMutex()
        self.running = False

    def terminate(self) -> None:
        self.mutex.unlock()
        self.running = False
        return super().terminate()
    
    def isRunning(self) -> bool:
        return self.running


    def run(self):
        self.mutex.lock()
        self.running = True
        # 或者 config = cfg.config.model_copy()
        try:
            self.search(cfg.config.model_dump())
        except Exception as e:
            log.error(f"Search failed: {e}")
        finally:
            print("Search thread finished.")
            self.mutex.unlock()
            self.running = False


    def search(self, config: dict):
            # 模拟搜索过程
            for i in range(200):
                self.msleep(100)  # 模拟耗时操作
                SearchMessages.searchFinish.emit(i + 1)  # 发出搜索完成信号
                log.info(f"搜索进度: {i + 1}/200")

            SearchMessages.searchEnd.emit()  # 发出搜索结束信号