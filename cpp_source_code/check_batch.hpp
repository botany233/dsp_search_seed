#pragma once
#include <pybind11/pybind11.h>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>
#include <windows.h>

#include "data_struct.hpp"
#include "check_seed.hpp"
#include "python_api.hpp"
#include "seed_manager.hpp"
#include "condition_to_struct.hpp"

namespace py = pybind11;

class GetDataQueue {
protected:
	struct Task {
		SeedStruct seed;
		int thread_num;
	};

	int max_cache;
	std::mutex task_mtx;
	std::queue<Task> tasks;
	std::condition_variable_any on_task_generated;
	bool stopped = false;

	std::mutex result_mtx;
	std::condition_variable_any on_result_clear;
	std::vector<GalaxyData> result;
	// Threads must be destroyed before the state they access.
	std::jthread worker_thread;

	void worker_func(std::stop_token token) {
		while(true) {
			Task task;
			{
				std::unique_lock lck(task_mtx);
				on_task_generated.wait(lck,token,[this] { return !tasks.empty(); });
				if(stopped || token.stop_requested())
					break;
				task = tasks.front();
				tasks.pop();
			}

			auto galaxy_data = get_galaxy_data_para(task.seed,task.thread_num);
			std::unique_lock lck(result_mtx);
			// An in-flight result is retained even when stopping with a full cache.
			on_result_clear.wait(lck,token,[this] { return result.size() < max_cache; });
			result.push_back(std::move(galaxy_data));
		}
	}
public:
	explicit GetDataQueue(int max_cache=1024): max_cache(std::max(max_cache,1)),
		worker_thread([this](std::stop_token token) { worker_func(token); }) {}

	~GetDataQueue() {
		shutdown();
	}

	void add_task(const SeedStruct& seed,int thread_num) {
		{
			std::lock_guard lck(task_mtx);
			if(stopped)
				return;
			tasks.push({seed,thread_num});
		}
		on_task_generated.notify_one();
	}

	void shutdown() {
		{
			std::lock_guard lck(task_mtx);
			stopped = true;
		}
		worker_thread.request_stop();
		if(worker_thread.joinable())
			worker_thread.join();
	}

	std::vector<GalaxyData> get_results() {
		std::vector<GalaxyData> return_result;
		{
			std::lock_guard lck(result_mtx);
			return_result.swap(result);
		}
		on_result_clear.notify_one();
		return return_result;
	}
};

class GetDataManager {
protected:
	std::mutex task_mtx;
	std::queue<SeedStruct> tasks;
	std::condition_variable_any on_task_generated;
	bool stopped = false;
	int max_thread;
	bool quick;
	int max_cache;

	std::mutex result_mtx;
	std::condition_variable_any on_result_clear;
	std::vector<GalaxyData> result;
	std::vector<std::jthread> search_threads;

	void search_func(std::stop_token token) {
		while(true) {
			SeedStruct current_task;
			{
				std::unique_lock lck(task_mtx);
				on_task_generated.wait(lck,token,[this] { return !tasks.empty(); });
				if(stopped || token.stop_requested())
					break;
				current_task = tasks.front();
				tasks.pop();
			}

			auto galaxy_data = get_galaxy_data(current_task,quick);
			std::unique_lock lck(result_mtx);
			on_result_clear.wait(lck,token,[this] { return result.size() < max_cache; });
			result.push_back(std::move(galaxy_data));
		}
	}
public:
	GetDataManager(int max_thread,bool quick,int max_cache=1024):
		max_thread(std::clamp(max_thread,1,128)),quick(quick),max_cache(std::max(max_cache,1))
	{
		try {
			search_threads.reserve(this->max_thread);
			for(int i=0;i<this->max_thread;++i)
				search_threads.emplace_back([this](std::stop_token token) { search_func(token); });
		} catch(...) {
			shutdown();
			throw;
		}
	}

	~GetDataManager() {
		shutdown();
	}

	void add_task(const SeedStruct& seed) {
		{
			std::lock_guard lck(task_mtx);
			if(stopped)
				return;
			tasks.push(seed);
		}
		on_task_generated.notify_one();
	}

	void shutdown() {
		{
			std::lock_guard lck(task_mtx);
			stopped = true;
		}
		for(auto& worker: search_threads)
			worker.request_stop();
		for(auto& worker: search_threads) {
			if(worker.joinable())
				worker.join();
		}
		search_threads.clear();
	}

	std::vector<GalaxyData> get_results() {
		std::vector<GalaxyData> return_result;
		{
			std::lock_guard lck(result_mtx);
			return_result.swap(result);
		}
		on_result_clear.notify_all();
		return return_result;
	}
};

class CheckPreciseManager {
protected:
	std::mutex task_mtx;
	std::queue<SeedStruct> tasks;
	std::condition_variable_any cv_generator;
	std::condition_variable_any cv_consumer;
	bool paused = false;
	bool stopped = false;
	bool started = false;
	bool finish = false;
	std::atomic<int> working_num = 0;

	std::uint8_t resource_index;
	SeedManager* seed_manager;
	GalaxyCondition galaxy_condition;
	int max_thread;
	int check_level;

	std::atomic<std::size_t> finish_task_num = 0;
	std::mutex result_mtx;
	std::vector<SeedStruct> result;
	std::vector<std::jthread> search_threads;
	std::jthread task_thread;

	void task_generator(std::stop_token token) {
		SetThreadPriorityBoost(GetCurrentThread(),TRUE);
		seed_manager->reset_index();
		while(true) {
			std::unique_lock lck(task_mtx);
			cv_generator.wait(lck,token,[this] { return !paused && tasks.size() <= 1024; });
			if(stopped || token.stop_requested())
				break;
			auto batch_seeds = seed_manager->get_seeds(1024,resource_index);
			if(batch_seeds.empty())
				break;
			for(const auto& seed: batch_seeds)
				tasks.push(seed);
			lck.unlock();
			cv_consumer.notify_all();
		}
		{
			std::lock_guard lck(task_mtx);
			finish = true;
		}
		cv_consumer.notify_all();
	}

	void search_func(std::stop_token token) {
		SetThreadPriorityBoost(GetCurrentThread(),TRUE);
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
		while(true) {
			SeedStruct current_task;
			{
				std::unique_lock lck(task_mtx);
				cv_consumer.wait(lck,token,[this] { return !paused && (!tasks.empty() || finish); });
				if(stopped || token.stop_requested() || (tasks.empty() && finish))
					break;
				current_task = tasks.front();
				tasks.pop();
			}
			cv_generator.notify_one();
			if(check_seed(current_task,galaxy_condition,check_level)) {
				std::lock_guard lck(result_mtx);
				result.push_back(current_task);
			}
			finish_task_num.fetch_add(1);
		}
		working_num.fetch_sub(1);
	}
public:
	CheckPreciseManager(SeedManager& seed_manager,std::uint8_t resource_index,
		const py::dict& galaxy_condition_dict,bool quick,int max_thread):
		resource_index(resource_index),seed_manager(&seed_manager),
		galaxy_condition(galaxy_condition_to_struct(galaxy_condition_dict)),
		max_thread(std::clamp(max_thread,1,128)),check_level(get_condition_level(galaxy_condition,quick)) {}

	~CheckPreciseManager() {
		shutdown();
	}

	void start_wait() {
		std::lock_guard lck(task_mtx);
		paused = true;
	}

	void end_wait() {
		{
			std::lock_guard lck(task_mtx);
			paused = false;
		}
		cv_generator.notify_all();
		cv_consumer.notify_all();
	}

	void run() {
		{
			std::lock_guard lck(task_mtx);
			if(started || stopped)
				return;
			started = true;
		}
		// Count all workers before any can finish, including those not yet created.
		working_num.store(max_thread);
		try {
			search_threads.reserve(max_thread);
			task_thread = std::jthread([this](std::stop_token token) { task_generator(token); });
			for(int i=0;i<max_thread;++i)
				search_threads.emplace_back([this](std::stop_token token) { search_func(token); });
		} catch(...) {
			working_num.fetch_sub(max_thread - static_cast<int>(search_threads.size()));
			shutdown();
			throw;
		}
	}

	bool is_running() {
		return working_num.load() > 0;
	}

	void shutdown() {
		{
			std::lock_guard lck(task_mtx);
			stopped = true;
		}
		task_thread.request_stop();
		for(auto& worker: search_threads)
			worker.request_stop();
		if(task_thread.joinable())
			task_thread.join();
		for(auto& worker: search_threads) {
			if(worker.joinable())
				worker.join();
		}
		search_threads.clear();
	}

	std::size_t get_task_num() {
		return seed_manager->get_seeds_count();
	}

	std::size_t get_task_progress() {
		return finish_task_num.load();
	}

	std::size_t get_result_num() {
		std::lock_guard lck(result_mtx);
		return result.size();
	}

	SeedStruct get_last_result() {
		std::lock_guard lck(result_mtx);
		return result.empty() ? SeedStruct(-1,-1,0) : result.back();
	}

	std::vector<SeedStruct> get_results() {
		std::lock_guard lck(result_mtx);
		return result;
	}
};

class CheckBatchManager {
protected:
	std::atomic<int> working_num = 0;
	std::atomic<std::size_t> task_id = 0;
	std::atomic<std::size_t> finish_task_num = 0;
	std::mutex task_mtx;
	std::condition_variable_any cv_resume;
	bool paused = false;
	bool stopped = false;
	bool started = false;

	GalaxyCondition galaxy_condition;
	int check_level;
	int start_seed;
	int start_star_num;
	int star_num_count;
	std::uint8_t resource_index;
	std::size_t task_num;
	int max_thread;

	std::mutex result_mtx;
	std::vector<SeedStruct> result;
	std::vector<std::jthread> search_threads;

	void search_func(std::stop_token token) {
		SetThreadPriorityBoost(GetCurrentThread(),TRUE);
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
		while(true) {
			std::size_t current_task_id;
			{
				std::unique_lock lck(task_mtx);
				cv_resume.wait(lck,token,[this] { return !paused; });
				if(stopped || token.stop_requested())
					break;
				current_task_id = task_id.fetch_add(1);
			}
			if(current_task_id >= task_num)
				break;

			const int seed_id = start_seed + static_cast<int>(current_task_id / star_num_count);
			const int star_num = start_star_num + static_cast<int>(current_task_id % star_num_count);
			const SeedStruct task(seed_id,star_num,resource_index);
			if(check_seed(task,galaxy_condition,check_level)) {
				std::lock_guard lck(result_mtx);
				result.push_back(task);
			}
			finish_task_num.fetch_add(1);
		}
		working_num.fetch_sub(1);
	}
public:
	CheckBatchManager(int start_seed,int end_seed,int start_star_num,int end_star_num,std::uint8_t resource_index,
		const py::dict& galaxy_condition_dict,bool quick,int max_thread):
		CheckBatchManager(start_seed,end_seed,start_star_num,end_star_num,resource_index,
			galaxy_condition_to_struct(galaxy_condition_dict),quick,max_thread) {}

	CheckBatchManager(int start_seed,int end_seed,int start_star_num,int end_star_num,std::uint8_t resource_index,
		const GalaxyCondition& galaxy_condition,bool quick,int max_thread):
		galaxy_condition(galaxy_condition),check_level(get_condition_level(galaxy_condition,quick)),
		start_seed(start_seed),start_star_num(start_star_num),star_num_count(std::max(end_star_num-start_star_num,0)),
		resource_index(resource_index),task_num(std::size_t(std::max(end_seed-start_seed,0)) * star_num_count),
		max_thread(std::clamp(max_thread,1,128)) {}

	~CheckBatchManager() {
		shutdown();
	}

	void start_wait() {
		std::lock_guard lck(task_mtx);
		paused = true;
	}

	void end_wait() {
		{
			std::lock_guard lck(task_mtx);
			paused = false;
		}
		cv_resume.notify_all();
	}

	void run() {
		{
			std::lock_guard lck(task_mtx);
			if(started || stopped)
				return;
			started = true;
		}
		working_num.store(max_thread);
		try {
			search_threads.reserve(max_thread);
			for(int i=0;i<max_thread;++i)
				search_threads.emplace_back([this](std::stop_token token) { search_func(token); });
		} catch(...) {
			working_num.fetch_sub(max_thread - static_cast<int>(search_threads.size()));
			shutdown();
			throw;
		}
	}

	bool is_running() {
		return working_num.load() > 0;
	}

	void shutdown() {
		{
			std::lock_guard lck(task_mtx);
			stopped = true;
		}
		for(auto& worker: search_threads)
			worker.request_stop();
		for(auto& worker: search_threads) {
			if(worker.joinable())
				worker.join();
		}
		search_threads.clear();
	}

	std::size_t get_task_num() const {
		return task_num;
	}

	std::size_t get_task_progress() {
		return finish_task_num.load();
	}

	std::size_t get_result_num() {
		std::lock_guard lck(result_mtx);
		return result.size();
	}

	SeedStruct get_last_result() {
		std::lock_guard lck(result_mtx);
		return result.empty() ? SeedStruct(-1,0,0) : result.back();
	}

	std::vector<SeedStruct> get_results() {
		std::lock_guard lck(result_mtx);
		return result;
	}
};
