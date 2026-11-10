from PySide6.QtCore import QMutex, QMutexLocker

class SeedList():
    def __init__(self, max_seed: int):
        self.seed_list = {}
        self.max_seed = max_seed
        self.mutex = QMutex()

    def set_seed_value(self, seed_id: int, star_num: int, sort_value: float|int) -> None:
        locker = QMutexLocker(self.mutex)
        if (seed_id, star_num) in self.seed_list:
            self.seed_list[(seed_id, star_num)] = sort_value

    def get_all_data(self) -> list[tuple[int, int, float|int]]:
        locker = QMutexLocker(self.mutex)
        ret = [(seed_id, star_num, sort_value) for (seed_id, star_num), sort_value in self.seed_list.items()]
        return ret

    def get_single_data(self, seed_id: int, star_num: int) -> float|int|None:
        locker = QMutexLocker(self.mutex)
        if (seed_id, star_num) in self.seed_list:
            return self.seed_list[(seed_id, star_num)]
        else:
            return None

    def is_exist(self, seed_id: int, star_num: int) -> bool:
        locker = QMutexLocker(self.mutex)
        ret = (seed_id, star_num) in self.seed_list
        return ret

    def add_seed(self, seed_id: int, star_num: int, check_limit: bool = True) -> bool:
        if not (0 <= seed_id <= 99999999 and 32 <= star_num <= 64):
            return False
        locker = QMutexLocker(self.mutex)
        if check_limit and len(self.seed_list) >= self.max_seed:
            return False
        if (seed_id, star_num) in self.seed_list:
            return False
        else:
            self.seed_list[(seed_id, star_num)] = 0
            return True

    def del_seeds(self, data: list[tuple[int, int]]) -> None:
        locker = QMutexLocker(self.mutex)
        for seed_id, star_num in data:
            if (seed_id, star_num) in self.seed_list:
                del self.seed_list[(seed_id, star_num)]

    def get_seed_num(self) -> tuple[int, int]:
        locker = QMutexLocker(self.mutex)
        ret = len(self.seed_list)
        return ret, self.max_seed

    def is_full(self) -> bool:
        cur_num, max_num = self.get_seed_num()
        return cur_num >= max_num
