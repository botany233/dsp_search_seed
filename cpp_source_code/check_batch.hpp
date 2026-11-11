#pragma once
#include <pybind11/pybind11.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>
#include <windows.h>

#include "data_struct.hpp"
#include "check_seed.hpp"
#include "astro_class.hpp"
#include "get_galaxy_data.hpp"
#include "python_api.hpp"
#include "seed_manager.hpp"
#include "condition_struct.hpp"
#include "src/DotNet35Random.hpp"

namespace py = pybind11;

// 线程仅在学习结束时提交一次；最后一个提交者旋转共享条件并发布结果。
struct CoreOrderLearning {
	struct Observation {
		std::size_t count = 0;
		std::uint64_t nanoseconds = 0;
	};
	struct Worker {
		std::vector<Observation> observations;
		DotNet35RandomLong random;

		explicit Worker(std::size_t core_count): observations(core_count),random(entropy_seed()) {}

		bool check(const SeedStruct& seed,const ConditionManager& conditions,bool quick,GalaxyClassSimple& workspace) {
			const std::size_t start = static_cast<std::size_t>(random.Next(static_cast<int>(observations.size())));
			const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			const bool passed = check_seed(seed,conditions,quick,workspace,start);
			const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			++observations[start].count;
			observations[start].nanoseconds += static_cast<std::uint64_t>(
				std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count());
			return passed;
		}

		static int entropy_seed() {
			// 使用 MSVC 的系统随机源，屏蔽符号位以避开 abs(INT_MIN)。
			std::random_device entropy;
			return static_cast<int>(entropy() & 0x7FFFFFFFu);
		}
	};
	std::size_t budget = 0;
	int submitted = 0;
	bool frozen = false;
	std::vector<Observation> observations;
	std::mutex mtx;
	std::condition_variable_any on_frozen;

	void initialize(std::size_t task_count,std::size_t core_count,bool quick) {
		if(core_count > 1 && task_count > 5000 * core_count && task_count > (quick ? 100000u : 10000u)) {
			budget = 2000 * core_count;
			observations.resize(core_count);
		}
	}

	bool synchronize(const std::vector<Observation>& local,int worker_count,ConditionManager& conditions,std::stop_token token) {
		std::unique_lock lck(mtx);
		for(std::size_t i=0;i<local.size();++i) {
			observations[i].count += local[i].count;
			observations[i].nanoseconds += local[i].nanoseconds;
		}
		if(++submitted == worker_count) {
			std::size_t count = 0,best = observations.size();
			for(std::size_t i=0;i<observations.size();++i) {
				const Observation& candidate = observations[i];
				count += candidate.count;
				if(candidate.count && (best == observations.size() ||
					static_cast<long double>(candidate.nanoseconds) / candidate.count <
					static_cast<long double>(observations[best].nanoseconds) / observations[best].count))
					best = i;
			}
			// 精筛输入提前耗尽时保留原序；停止时也不提交新顺序。
			if(count == budget && best != observations.size() && !token.stop_requested())
				conditions.rotate_core_conditions(best);
			frozen = true;
			lck.unlock();
			on_frozen.notify_all();
			return !token.stop_requested();
		}
		return on_frozen.wait(lck,token,[this] { return frozen; }) && !token.stop_requested();
	}
};

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

class GetDataManagerFast {
protected:
	std::mutex task_mtx;
	std::queue<SeedStruct> tasks;
	std::condition_variable_any on_task_generated;
	bool stopped = false;
	int max_thread;
	bool quick;
	int max_cache;
	const int gen_planet_num;
	const unsigned short vein_mask;

	std::mutex result_mtx;
	std::condition_variable_any on_result_clear;
	std::vector<GalaxyDataSimple> result;
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

			GalaxyData galaxy = get_galaxy_data_fast(current_task,quick,gen_planet_num,vein_mask);
			GalaxyDataSimple galaxy_simple;
			galaxy_simple.seed_id = galaxy.seed_id;
			galaxy_simple.star_num = galaxy.star_num;
			galaxy_simple.resource_index = galaxy.resource_index;
			for(int i=0;i<14;i++) {
				galaxy_simple.veins_point[i] = galaxy.veins_point[i];
				galaxy_simple.veins_amount[i] = galaxy.veins_amount[i];
			}
			for(const StarData& star: galaxy.stars) {
				galaxy_simple.star_type_num[star.type_id]++;
				for(const PlanetData& planet: star.planets)
					galaxy_simple.planet_type_num[planet.type_id]++;
			}
			std::unique_lock lck(result_mtx);
			on_result_clear.wait(lck,token,[this] { return result.size() < max_cache; });
			result.push_back(std::move(galaxy_simple));
		}
	}
public:
	GetDataManagerFast(int max_thread,bool quick,int max_cache=1024,int gen_planet_num=-1,
		unsigned short vein_mask=0x3FFF):
		max_thread(std::clamp(max_thread,1,128)),quick(quick),max_cache(std::max(max_cache,1)),
		gen_planet_num(gen_planet_num),vein_mask(vein_mask)
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

	~GetDataManagerFast() {
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

	std::vector<GalaxyDataSimple> get_results() {
		std::vector<GalaxyDataSimple> return_result;
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
	std::size_t task_id = 0; // 由 task_mtx 保护，与实际出队顺序一致。
	ConditionManager galaxy_condition;
	CoreOrderLearning order_learning;
	int max_thread;
	bool quick;

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
		GalaxyClassSimple workspace;
		const std::size_t budget = order_learning.budget;
		bool learning = budget != 0;
		std::optional<CoreOrderLearning::Worker> learner;
		if(learning)
			learner.emplace(galaxy_condition.core_conditions.size());
		while(true) {
			SeedStruct current_task;
			std::size_t current_task_id = 0;
			bool exhausted;
			{
				std::unique_lock lck(task_mtx);
				cv_consumer.wait(lck,token,[this,&learning,budget] {
					return !paused && (!tasks.empty() || finish || (learning && task_id >= budget));
				});
				if(stopped || token.stop_requested())
					break;
				if(learning)
					current_task_id = task_id;
				exhausted = tasks.empty() && finish;
				if(!exhausted && (!learning || current_task_id < budget)) {
					current_task = tasks.front();
					tasks.pop();
					if(learning)
						++task_id;
				}
			}
			if(learning && (exhausted || current_task_id >= budget)) {
				if(!order_learning.synchronize(learner->observations,max_thread,galaxy_condition,token))
					break;
				learning = false;
				continue;
			}
			if(exhausted)
				break;
			cv_generator.notify_one();
			const bool passed = learning ? learner->check(current_task,galaxy_condition,quick,workspace) :
				check_seed(current_task,galaxy_condition,quick,workspace);
			if(passed) {
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
		CheckPreciseManager(seed_manager,resource_index,ConditionManager(galaxy_condition_dict),quick,max_thread) {}

	CheckPreciseManager(SeedManager& seed_manager,std::uint8_t resource_index,
		const ConditionManager& galaxy_condition,bool quick,int max_thread):
		resource_index(resource_index),seed_manager(&seed_manager),galaxy_condition(galaxy_condition),
		max_thread(std::clamp(max_thread,1,128)),quick(quick) {}

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
			order_learning.initialize(seed_manager->get_seeds_count(),galaxy_condition.core_conditions.size(),quick);
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
	std::size_t task_id = 0; // 领取任务时始终持有 task_mtx，无需额外原子操作。
	std::atomic<std::size_t> finish_task_num = 0;
	std::mutex task_mtx;
	std::condition_variable_any cv_resume;
	bool paused = false;
	bool stopped = false;
	bool started = false;

	ConditionManager galaxy_condition;
	CoreOrderLearning order_learning;
	bool quick;
	std::uint8_t max_position;
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
		GalaxyClassSimple workspace;
		workspace.max_position = max_position;
		const std::size_t budget = order_learning.budget;
		bool learning = budget != 0;
		std::optional<CoreOrderLearning::Worker> learner;
		if(learning)
			learner.emplace(galaxy_condition.core_conditions.size());
		while(true) {
			std::size_t current_task_id;
			{
				std::unique_lock lck(task_mtx);
				cv_resume.wait(lck,token,[this] { return !paused; });
				if(stopped || token.stop_requested())
					break;
				current_task_id = task_id;
				if(!learning || current_task_id < budget)
					++task_id;
			}
			if(learning && current_task_id >= budget) {
				if(!order_learning.synchronize(learner->observations,max_thread,galaxy_condition,token))
					break;
				learning = false;
				continue;
			}
			if(current_task_id >= task_num)
				break;

			const int seed_id = start_seed + static_cast<int>(current_task_id / star_num_count);
			const int star_num = start_star_num + static_cast<int>(current_task_id % star_num_count);
			const SeedStruct task(seed_id,star_num,resource_index);
			const bool passed = learning ? learner->check(task,galaxy_condition,quick,workspace) :
				check_seed(task,galaxy_condition,quick,workspace);
			if(passed) {
				std::lock_guard lck(result_mtx);
				result.push_back(task);
			}
			finish_task_num.fetch_add(1);
		}
		working_num.fetch_sub(1);
	}
public:
	// 非空批次仅在遍历多个恒星数时按最大实际星数缓存坐标。
	CheckBatchManager(int start_seed,int end_seed,int start_star_num,int end_star_num,std::uint8_t resource_index,
		const py::dict& galaxy_condition_dict,bool quick,int max_thread):
		CheckBatchManager(start_seed,end_seed,start_star_num,end_star_num,resource_index,
			ConditionManager(galaxy_condition_dict),quick,max_thread) {}

	CheckBatchManager(int start_seed,int end_seed,int start_star_num,int end_star_num,std::uint8_t resource_index,
		const ConditionManager& galaxy_condition,bool quick,int max_thread):
		galaxy_condition(galaxy_condition),quick(quick),
		max_position(start_seed < end_seed && end_star_num - start_star_num > 1 ?
			static_cast<std::uint8_t>(end_star_num - 1) : 0),
		start_seed(start_seed),start_star_num(start_star_num),star_num_count(std::max(end_star_num-start_star_num,0)),
		resource_index(resource_index),task_num(std::size_t(std::max(end_seed-start_seed,0)) * star_num_count),
		max_thread(std::clamp(max_thread,1,128)) {
		order_learning.initialize(task_num,this->galaxy_condition.core_conditions.size(),quick);
	}

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
