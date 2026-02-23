#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <thread>
#include <atomic>
#include <chrono>

#include "astro_class.hpp"
#include "PlanetAlgorithm.hpp"
#include "DotNet35Random.hpp"

using namespace std;
namespace py = pybind11;

class GPUBenchmark {
protected:
	vector<thread> threads{};
	atomic<bool> stop = false;
	atomic<int> num = 0;

	int max_thread;

	void main_func(int seed) {
		DotNet35Random rand_gen = DotNet35Random(seed);
		while(true) {
			if(stop.load()) break;
			
			PlanetClassSimple planet;
			planet.seed = rand_gen.Next();
			planet.mod_x = rand_gen.NextDouble();
			planet.mod_y = rand_gen.NextDouble();

			auto planet_algo = PlanetAlgorithmManager(rand_gen.Next(1,14));
			planet_algo->GenerateTerrain(planet);
			num.fetch_add(1);
		}
	}
public:
	GPUBenchmark(int max_thread)
	{
		this->max_thread = max_thread;
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

	double do_test(double test_time) {
		num.store(0);
		auto start = chrono::steady_clock::now();
		this_thread::sleep_for(chrono::milliseconds((int)(test_time * 1000)));
		double finish_num = (double)num.load();
		auto end = chrono::steady_clock::now();
		double use_time = (double)chrono::duration_cast<chrono::microseconds>(end-start).count();
		double speed = 1000000 * finish_num / use_time;
		return speed;
	}
};
