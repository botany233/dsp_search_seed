#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <thread>
#include <atomic>
#include <chrono>

#include "check_seed.hpp"
#include "DotNet35Random.hpp"

using namespace std;
namespace py = pybind11;

class GPUBenchmark {
protected:
	vector<thread> threads{};
	atomic<bool> stop = false;
	atomic<int> num = 0;

	chrono::steady_clock::time_point tag;

	int max_thread;
	bool heavy;

	void main_func(int seed) {
		DotNet35Random rand_gen = DotNet35Random(seed);
		while(true) {
			if(stop.load()) break;

			SeedStruct seed = SeedStruct(rand_gen.Next(0,100000000),32,rand_gen.Next(0,11));
			GalaxyData galaxy;
			if(heavy)
				galaxy = get_galaxy_data(seed,false);
			else
				galaxy = get_galaxy_data_fast(seed,false);
			int planet_num = 0;
			for(const StarData& star: galaxy.stars)
				for(const PlanetData& planet: star.planets)
					if(!planet.is_gas)
						planet_num++;
			num.fetch_add(planet_num);
		}
	}
public:
	GPUBenchmark(int max_thread,bool heavy) {
		this->max_thread = max_thread;
		this->heavy = heavy;
	}

	~GPUBenchmark() {
		shutdown();
	}

	void run() {
		int seed = static_cast<int>(time(nullptr));
		DotNet35Random rand_gen = DotNet35Random(seed);
		for(int i=0;i<max_thread;i++) {
			threads.push_back(thread(&GPUBenchmark::main_func,this,rand_gen.Next()));
		}
	}

	void shutdown() {
		stop.store(true);
		for(int i=0;i<max_thread;i++) {
			thread& main_thread = threads[i];
			if(main_thread.joinable())
				main_thread.join();
		}
	}

	void reset() {
		tag = chrono::steady_clock::now();
		num.store(0);
	}

	double get_speed() {
		auto now = chrono::steady_clock::now();
		double cur_num = (double)num.load();
		double use_time = (double)chrono::duration_cast<chrono::microseconds>(now-tag).count();
		double speed = 1000000 * cur_num / use_time;
		return speed;
	}
};
