#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <memory>

#include "data_struct.hpp"
#include "check_seed.hpp"
#include "python_api.hpp"
#include "seed_manager.hpp"
#include "condition_to_struct.hpp"

using namespace std;
namespace py = pybind11;

class GetDataManager {
protected:
	vector<thread> search_threads{};
	mutex task_mtx;
	queue<SeedStruct> tasks{};
	condition_variable on_task_generated;
	atomic<bool> stop = false;

	int max_thread;
	bool quick;
	int max_cache;

	mutex result_mtx;
	condition_variable on_result_clear;
	vector<GalaxyData> result{};

	void search_func() {
		while(true) {
			unique_lock<mutex> task_lck(task_mtx);
			on_task_generated.wait(task_lck,[this]() { return !tasks.empty() || stop.load(); });
			if(stop.load()) {
				task_lck.unlock();
				break;
			}
			SeedStruct current_task = tasks.front();
			tasks.pop();
			task_lck.unlock();

			GalaxyData galaxy_data = get_galaxy_data(current_task.seed_id,current_task.star_num,quick);
			unique_lock<mutex> lck(result_mtx);
			on_result_clear.wait(lck,[this]() { return result.size() < max_cache || stop.load(); });
			result.push_back(galaxy_data);
		}
	}
public:
	GetDataManager(int max_thread,bool quick,int max_cache=1024)
	{
		this->max_thread = max_thread;
		this->quick = quick;
		this->max_cache = max_cache;
		for(int i=0;i<max_thread;i++) {
			search_threads.push_back(thread(&GetDataManager::search_func,this));
		}
	}

	~GetDataManager() {
		shutdown();
	}

	void add_task(int seed_id,int star_num) {
		{
			lock_guard<mutex> lck(task_mtx);
			tasks.emplace(seed_id,star_num);
		}
		on_task_generated.notify_one();
	}

	void shutdown() {
		stop.store(true);
		on_task_generated.notify_all();
		on_result_clear.notify_all();
		for(int i=0;i<max_thread;i++) {
			thread& search_thread = search_threads[i];
			if(search_thread.joinable())
				search_thread.join();
		}
		//search_threads.clear();
	}

	vector<GalaxyData> get_results() {
		vector<GalaxyData> return_result;
		{
			lock_guard<mutex> lck(result_mtx);
			return_result = move(result);
			result.clear();
		}
		on_result_clear.notify_all();
		return return_result;
	}
};

class CheckPreciseManager {
protected:
	vector<thread> search_threads;
	thread task_thread;
	mutex task_mtx;
	queue<SeedStruct> tasks;
	atomic<int> working_num = 0;
	atomic<size_t> finish_task_num = 0;
	atomic<bool> finish = false;
	atomic<bool> stop = false;

	SeedManager seed_manager = SeedManager();
	GalaxyCondition galaxy_condition;
	int max_thread;
	int check_level;

	mutex result_mtx;
	vector<SeedStruct> result = vector<SeedStruct>(0);

	void task_generator() {
		seed_manager.reset_index();
		while(true) {
			while(tasks.size() > 2048 && !stop.load())
				this_thread::sleep_for(chrono::milliseconds(20));
			if(stop.load())
				break;
			vector<SeedStruct> batch_seeds = seed_manager.get_seeds(1024);
			if(batch_seeds.size()==0)
				break;
			lock_guard<mutex> lck(task_mtx);
			for(const SeedStruct& seed: batch_seeds) {
				tasks.push(seed);
			}
		}
		finish.store(true);
	}

	void search_func() {
		while(true) {
			bool is_empty = false;
			SeedStruct current_task;
			{
				lock_guard<mutex> lck(task_mtx);
				if((tasks.empty() && finish.load()) || stop.load()) {
					break;
				}
				is_empty = tasks.empty();
				if(!is_empty) {
					current_task = tasks.front();
					tasks.pop();
				}
			}
			if(is_empty) {
				this_thread::sleep_for(chrono::milliseconds(20));
				continue;
			}
			if(check_seed_level_1(current_task.seed_id,current_task.star_num,galaxy_condition,check_level)) {
				lock_guard<mutex> lck(result_mtx);
				result.push_back(current_task);
			}
			finish_task_num.fetch_add(1);
		}
		working_num.fetch_add(-1);
	}
public:
	CheckPreciseManager(SeedManager& seed_manager,
		const py::dict& galaxy_condition_dict,bool quick,int max_thread)
	{
		galaxy_condition = galaxy_condition_to_struct(galaxy_condition_dict);
		check_level = get_condition_level(galaxy_condition,quick);
		this->max_thread = max_thread;
		this->seed_manager.set_raw_data(seed_manager.get_raw_data());
	}

	~CheckPreciseManager() {
		shutdown();
	}

	void run() {
		task_thread = thread(&CheckPreciseManager::task_generator,this);
		for(int i=0;i<max_thread;i++) {
			working_num.fetch_add(1);
			search_threads.push_back(thread(&CheckPreciseManager::search_func,this));
		}
	}

	bool is_running() {
		return working_num.load() > 0;
	}

	void shutdown() {
		stop.store(true);
		if(task_thread.joinable())
			task_thread.join();
		task_thread = thread();
		for(int i=0;i<max_thread;i++) {
			thread& search_thread = search_threads[i];
			if(search_thread.joinable())
				search_thread.join();
		}
		//search_threads.clear();
	}

	size_t get_task_num() {
		return seed_manager.get_seeds_count();
	}

	size_t get_task_progress() {
		return finish_task_num.load();
	}

	size_t get_result_num() {
		lock_guard<mutex> lck(result_mtx);
		return result.size();
	}

	SeedStruct get_last_result() {
		lock_guard<mutex> lck(result_mtx);
		if(result.empty())
			return SeedStruct(-1,-1);
		return result.back();
	}

	vector<SeedStruct> get_results() {
		lock_guard<mutex> lck(result_mtx);
		return result;
	}
};

class CheckBatchManager {
protected:
	vector<thread> search_threads;
	atomic<int> working_num = 0;
	atomic<size_t> task_id = 0;
	atomic<size_t> finish_task_num = 0;
	atomic<bool> stop = false;

	GalaxyCondition galaxy_condition;
	int check_level;
	int start_seed;
	int end_seed;
	int start_star_num;
	int end_star_num;
	size_t task_num;
	int max_thread;

	mutex mtx;
	vector<SeedStruct> result = vector<SeedStruct>(0);

	void search_func() {
		while(true) {
			size_t current_task_id = task_id.fetch_add(1);
			if(current_task_id >= task_num || stop.load()) {
				break;
			}

			int seed_id = start_seed + current_task_id / (end_star_num - start_star_num);
			int star_num = start_star_num + current_task_id % (end_star_num - start_star_num);

			if(check_seed_level_1(seed_id,star_num,galaxy_condition,check_level)) {
				lock_guard<mutex> lck(mtx);
				result.emplace_back(seed_id,star_num);
			}
			finish_task_num.fetch_add(1);
		}
		working_num.fetch_add(-1);
	}
public:
	CheckBatchManager(int start_seed,int end_seed,int start_star_num,int end_star_num,
		const py::dict& galaxy_condition_dict,bool quick,int max_thread)
	{
		galaxy_condition = galaxy_condition_to_struct(galaxy_condition_dict);
		check_level = get_condition_level(galaxy_condition,quick);
		this->start_seed = start_seed;
		this->end_seed = end_seed;
		this->start_star_num = start_star_num;
		this->end_star_num = end_star_num;
		this->max_thread = max_thread;
		task_num = (end_seed - start_seed) * (end_star_num - start_star_num);
	}

	CheckBatchManager(int start_seed,int end_seed,int start_star_num,int end_star_num,
		const GalaxyCondition& galaxy_condition,bool quick,int max_thread)
	{
		this->galaxy_condition = galaxy_condition;
		check_level = get_condition_level(galaxy_condition,quick);
		this->start_seed = start_seed;
		this->end_seed = end_seed;
		this->start_star_num = start_star_num;
		this->end_star_num = end_star_num;
		this->max_thread = max_thread;
		task_num = (end_seed - start_seed) * (end_star_num - start_star_num);
	}

	~CheckBatchManager() {
		shutdown();
	}

	void run() {
		for(int i=0;i<max_thread;i++) {
			working_num.fetch_add(1);
			search_threads.push_back(thread(&CheckBatchManager::search_func,this));
		}
	}

	bool is_running() {
		return working_num.load() > 0;
	}

	void shutdown() {
		stop.store(true);
		for(int i=0;i<max_thread;i++) {
			thread& search_thread = search_threads[i];
			if(search_thread.joinable())
				search_thread.join();
		}
		search_threads.clear();
	}

	size_t get_task_num() {
		return task_num;
	}

	size_t get_task_progress() {
		return finish_task_num.load();
	}

	size_t get_result_num() {
		lock_guard<mutex> lck(mtx);
		return result.size();
	}

	SeedStruct get_last_result() {
		lock_guard<mutex> lck(mtx);
		if(result.empty())
			return SeedStruct(-1,-1);
		return result.back();
	}

	vector<SeedStruct> get_results() {
		lock_guard<mutex> lck(mtx);
		return result;
	}
};
