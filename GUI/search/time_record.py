from time import perf_counter

class TimeRecord:
    def __init__(self, do_record: bool = True):
        self.do_record = do_record
        self.time_tag = perf_counter()
        self.total_time = 0

    def get_time(self) -> float:
        if self.do_record:
            self._record_time()
        return self.total_time

    def _record_time(self):
        now = perf_counter()
        self.total_time += now - self.time_tag
        self.time_tag = now

    def pause(self):
        self.do_record = False
    
    def resume(self):
        if not self.do_record:
            self.do_record = True
            self.time_tag = perf_counter()
