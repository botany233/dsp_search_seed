from typing import Generator
from bitarray import bitarray

class SeedManager:
    def __init__(self):
        self.records = [bitarray(10**8) for _ in range(32, 65)]

    def add_seed(self, seed: int, star_num: int) -> None:
        self.records[star_num - 32][seed] = 1

    def del_seed(self, seed: int, star_num: int) -> None:
        self.records[star_num - 32][seed] = 0

    def get_all_seeds(self, batch_size: int) -> Generator[tuple[list[int], list[int]], None, None]:
        current_seed, current_star_num = [], []
        num = 0
        for star_num in range(32, 65):
            ba = self.records[star_num - 32]
            for seed in ba.search(bitarray('1')):
                current_seed.append(seed)
                current_star_num.append(star_num)
                num += 1
                if num >= batch_size:
                    yield current_seed, current_star_num
                    current_seed, current_star_num = [], []
                    num = 0
        if num > 0:
            yield current_seed, current_star_num

    def get_seeds_count(self) -> int:
        return sum(i.count() for i in self.records)
    
    def clear(self) -> None:
        for ba in self.records:
            ba.setall(0)