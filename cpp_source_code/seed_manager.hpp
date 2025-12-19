#pragma once
#include <cstdint>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <intrin.h>
#include <iostream>

#include "data_struct.hpp"

using namespace std;

class SeedManager {
protected:
	static constexpr int LENGTH = 51562500;
	shared_mutex data_mtx;
	mutex index_mtx;
	vector<uint64_t> seeds;
	size_t seed_num = 0;
	int current_index = 0;
public:
	SeedManager() {
		seeds = vector<uint64_t>(LENGTH,0);
	}

	void add_seed(int seed_id,int star_num) {
		int64_t index = (int64_t)seed_id * 33 + star_num - 32;
		uint64_t mask = (uint64_t)1 << (index % 64);
		lock_guard<shared_mutex> lck(data_mtx);
		uint64_t& seed_entry = seeds[index / 64];
		if((seed_entry & mask) == 0) {
			seed_entry |= mask;
			seed_num++;
		}
	}
	
	void del_seed(int seed_id,int star_num) {
		int64_t index = seed_id * 33 + star_num - 32;
		uint64_t mask = uint64_t(1) << (index % 64);
		lock_guard<shared_mutex> lck(data_mtx);
		uint64_t& seed_entry = seeds[index / 64];
		if((seed_entry & mask) != 0) {
			seed_entry ^= mask;
			seed_num--;
		}
	}

	int64_t get_seeds_count() {
		shared_lock<shared_mutex> lck(data_mtx);
		return seed_num;
	}

	void clear() {
		lock_guard<shared_mutex> lck(data_mtx);
		seeds.assign(LENGTH,0);
	}

	void reset_index() {
		lock_guard<mutex> lck(index_mtx);
		current_index = 0;
	}

	vector<SeedStruct> get_seeds(int batch_num = 1) {
		vector<SeedStruct> result = vector<SeedStruct>(0);
		lock_guard<mutex> index_lck(index_mtx);
		shared_lock<shared_mutex> data_lck(data_mtx);
		while(current_index < LENGTH && result.size() < batch_num) {
			uint64_t& seed_entry = seeds[current_index];

			if(seed_entry != 0) {
				uint64_t temp = seed_entry;
				unsigned long bit_pos;
				while(_BitScanForward64(&bit_pos,temp)) {
					int64_t real_index = (int64_t)current_index * 64 + bit_pos;
					result.emplace_back(real_index/33,real_index%33+32);
					temp &= (temp - 1);
				}
			}
			current_index++;
		}
		return result;
	}

	pair<vector<uint64_t>,size_t> get_raw_data() {
		shared_lock<shared_mutex> lck(data_mtx);
		return pair(seeds,seed_num);
	}

	void set_raw_data(const pair<vector<uint64_t>,size_t>& raw_data) {
		lock_guard<shared_mutex> lck(data_mtx);
		seeds = raw_data.first;
		seed_num = raw_data.second;
	}
};
