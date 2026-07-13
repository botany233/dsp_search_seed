#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <intrin.h>
#include <iostream>

#include "data_struct.hpp"

using namespace std;

class SeedPage {
public:
	vector<uint64_t> seeds;
	size_t count = 0;

	bool add_seed(size_t seed_id) {
		if(count == 0)
			seeds.resize(15625,0);
		size_t word_idx = seed_id / 64;
		size_t bit_idx = seed_id % 64;
		uint64_t mask = uint64_t(1) << bit_idx;
		uint64_t& seed_entry = seeds[word_idx];
		if(seed_entry & mask)
			return false;

		seed_entry |= mask;
		count++;
		return true;
	}

	bool del_seed(size_t seed_id) {
		if(count == 0)
			return false;
		size_t word_idx = seed_id / 64;
		size_t bit_idx = seed_id % 64;
		uint64_t mask = uint64_t(1) << bit_idx;
		uint64_t& seed_entry = seeds[word_idx];
		if((seed_entry & mask) == 0)
			return false;
		seed_entry ^= mask;
		count--;
		if(count == 0)
			vector<uint64_t>().swap(seeds);
		return true;
	}

	void clear() {
		vector<uint64_t>().swap(seeds);
		count = 0;
	}
};

class SeedManager {
protected:
	mutex mtx;
	vector<SeedPage> pages;
	size_t seed_num = 0;
	int current_page = 0;
	int current_word = 0;

public:
	SeedManager() {
		pages.resize(3300);
	}

	bool add_seed(int seed_id,int star_num) {
		int page_idx = (seed_id / 1000000) * 33 + (star_num - 32);
		int offset = seed_id % 1000000;

		lock_guard<mutex> lck(mtx);
		SeedPage& page = pages[page_idx];
		return page.add_seed(offset);
	}
	
	bool del_seed(int seed_id,int star_num) {
		int page_idx = (seed_id / 1000000) * 33 + (star_num - 32);
		int offset = seed_id % 1000000;
		
		lock_guard<mutex> lck(mtx);
		SeedPage& page = pages[page_idx];
		return page.del_seed(offset);
	}

	size_t get_seeds_count() {
		lock_guard<mutex> lck(mtx);
		size_t seed_num = 0;
		for(const SeedPage& page : pages) {
			seed_num += page.count;
		}
		return seed_num;
	}

	void clear() {
		lock_guard<mutex> lck(mtx);
		for(SeedPage& page : pages) {
			page.clear();
		}
	}

	void reset_index() {
		lock_guard<mutex> lck(mtx);
		current_page = 0;
		current_word = 0;
	}

	vector<SeedStruct> get_seeds(size_t batch_num,uint8_t resource_index) {
		vector<SeedStruct> result;
		result.reserve(batch_num+64);

		lock_guard<mutex> lck(mtx);
		while(current_page < 3300) {
			const SeedPage& page = pages[current_page];
			const uint8_t star_num = current_page % 33 + 32;
			const int seed_id_fond = 1000000 * (current_page / 33);
			if(page.count > 0) {
				while(current_word < 15625) {
					uint64_t seed_entry = page.seeds[current_word];
					if(seed_entry != 0) {
						const int seed_id_base = seed_id_fond + current_word * 64;
						unsigned long offset;
						while(_BitScanForward64(&offset,seed_entry)) {
							result.emplace_back(seed_id_base + offset,star_num,resource_index);
							seed_entry &= (seed_entry - 1);
						}
					}
					current_word++;
					if(result.size() >= batch_num) {
						return result;
					}
				}
			}
			current_page++;
			current_word = 0;
		}
		return result;
	}
};
