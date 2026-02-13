#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <CL/opencl.hpp>
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdint>

#include "Vector3.hpp"
#include "python_api.hpp"
#include "check_seed.hpp"
#include "data_struct.hpp"
#include "seed_manager.hpp"
#include "PlanetRawData.hpp"
#include "defines.hpp"
#include "PlanetAlgorithm.hpp"
#include "util.hpp"
#include "check_batch.hpp"
#include "astro_class.hpp"

using namespace std;
using namespace chrono;
namespace py = pybind11;

static uint16_t get_need_veins(vector<int>& veins_group,vector<int>& veins_point) {
	uint16_t result = 0;
	for(int i=0;i<14;i++) {
		if(veins_point[i] > 0 || veins_group[i] > 0)
			result |= (1 << i);
	}
	return result;
}

static void add_need_veins(GalaxyCondition& galaxy_condition) {
	galaxy_condition.need_veins = get_need_veins(galaxy_condition.veins_group,galaxy_condition.veins_point);
	for(StarCondition& star_condition: galaxy_condition.stars) {
		star_condition.need_veins = get_need_veins(star_condition.veins_group,star_condition.veins_point);
		for(PlanetCondition& planet_condition: star_condition.planets) {
			planet_condition.need_veins = get_need_veins(planet_condition.veins_group,planet_condition.veins_point);
		}
	}
	for(PlanetCondition& planet_condition: galaxy_condition.planets) {
		planet_condition.need_veins = get_need_veins(planet_condition.veins_group,planet_condition.veins_point);
	}
}

GalaxyCondition mag_300_condition() {
	GalaxyCondition con;
	con.veins_point[13] = 300;
	return con;
}

GalaxyCondition satelite_light_condition() {
	GalaxyCondition con;
	PlanetCondition pc;
	pc.veins_point[11] = 50;
	pc.singularity = 1<<6;
	con.planets.push_back(pc);
	return con;
}

GalaxyCondition debug_veins_condition() {
	GalaxyCondition con;
	con.veins_point[6] = 200;
	con.veins_point[12] = 400;
	con.veins_point[13] = 200;
	return con;
}

GalaxyCondition debug_extreme_factory_condition() {
	GalaxyCondition con;
	con.veins_group[13] = 8;
	con.veins_point[12] = 400;
	StarCondition sc;
	sc.type = 14;
	sc.satisfy_num = 4;
	con.stars.push_back(sc);
	PlanetCondition pc;
	pc.type = 13;
	pc.satisfy_num = 8;
	con.planets.push_back(pc);
	return con;
}

GalaxyCondition good_birth_condition() {
	GalaxyCondition con;
	StarCondition sc;
	sc.distance = 10;
	sc.type = 14;
	for(int i=6;i<13;i++) {
		sc.veins_group[i] = 1;
	}
	con.stars.push_back(sc);
	return con;
}

//int main() {
//	system("chcp 65001>nul");
//	do_init();
//	set_device_id_c(0);
//
//	for(int seed=0;seed<1000;seed++){
//		GalaxyData galaxy_data = get_galaxy_data(seed,64,true);
//	}
//}

int main() {
	system("chcp 65001>nul");
	do_init();
	set_device_id_c(0);

	bool quick = false;
	//GalaxyCondition galaxy_condition = mag_300_condition();
	//GalaxyCondition galaxy_condition = satelite_light_condition();
	//GalaxyCondition galaxy_condition = debug_veins_condition();
	//GalaxyCondition galaxy_condition = debug_extreme_factory_condition();
	GalaxyCondition galaxy_condition = good_birth_condition();

	add_need_veins(galaxy_condition);
	int check_level = get_condition_level(galaxy_condition,quick);

	vector<string> results = check_batch(1752,1753,64,65,galaxy_condition,check_level);
	for(const string& result:results) {
		cout << result << endl;
	}
	cout << "找到 " << results.size() << " 个种子" << endl;
}

//int main() {
//	system("chcp 65001>nul");
//	do_init();
//	set_device_id_c(0);
//	GalaxyData galaxy_data = get_galaxy_data(5121007,64,false);
//	for(const StarData& star_data: galaxy_data.stars) {
//		cout << "star " << star_data.type << " " << star_data.name << endl;
//		for(const PlanetData& planet_data: star_data.planets) {
//			cout << "    planet " << planet_data.type << " " << planet_data.name << endl;
//		}
//	}
//}

// 测试gpu和cpu的星系生成效率
//int main() {
// system("chcp 65001>nul");
//	do_init();
//	set_device_id_c(0);
//
//	auto start = high_resolution_clock::now();
//	get_galaxy_data(233,64,false);
//	get_galaxy_data(1234,64,false);
//	auto end = high_resolution_clock::now();
//	auto duration = duration_cast<milliseconds>(end - start);
//	cout << "double gpu use time: " << duration.count() << " ms" << endl;
//
//	OpenCLManager::SUPPORT_DOUBLE = false;
//
//	start = high_resolution_clock::now();
//	get_galaxy_data(233,64,false);
//	get_galaxy_data(1234,64,false);
//	end = high_resolution_clock::now();
//	duration = duration_cast<milliseconds>(end - start);
//	cout << "float gpu use time: " << duration.count() << " ms" << endl;
//
//	OpenCLManager::SUPPORT_GPU = false;
//
//	start = high_resolution_clock::now();
//	get_galaxy_data(233,64,false);
//	get_galaxy_data(1234,64,false);
//	end = high_resolution_clock::now();
//	duration = duration_cast<milliseconds>(end - start);
//	cout << "cpu use time: " << duration.count() << " ms" << endl;
//	return 0;
//}

// 测试gpu和cpu的地形生成结果一致性
//int main()
//{
//	do_init();
//	set_device_id_c(0);
//	//OpenCLManager::SUPPORT_GPU = false;
//
//	double modX = 0.235;
//	double modY = 0.975;
//	int seed = 1485417518;
//
//	PlanetClass planet_data_gpu = PlanetClass();
//	planet_data_gpu.seed = seed;
//
//	auto planet_algorithm_gpu = PlanetAlgorithm1();
//	auto start = high_resolution_clock::now();
//	planet_algorithm_gpu.GenerateTerrain(planet_data_gpu,modX,modY);
//	auto end = high_resolution_clock::now();
//	auto duration = duration_cast<milliseconds>(end - start);
//	cout << "gpu use time: " << duration.count() << " ms" << endl;
//	
//	OpenCLManager::SUPPORT_GPU = false;
//
//	PlanetClass planet_data_cpu = PlanetClass();
//	planet_data_cpu.seed = seed;
//	
//	auto planet_algorithm_cpu = PlanetAlgorithm1();
//	start = high_resolution_clock::now();
//	planet_algorithm_cpu.GenerateTerrain(planet_data_cpu,modX,modY);
//	end = high_resolution_clock::now();
//	duration = duration_cast<milliseconds>(end - start);
//	cout << "cpu use time: " << duration.count() << " ms" << endl;
//
//	vector<unsigned short>& heightData_cpu = planet_data_cpu.data.heightData;
//	vector<unsigned short>& heightData_gpu = planet_data_gpu.data.heightData;
//
//	// 输出结果
//	for(int i=0;i<10;i++) {
//		cout << heightData_cpu[i] << " " << heightData_gpu[i] << endl;
//	}
//	cout << endl;
//
//	// 验证结果
//	double rel_error_sum = 0.0;
//	double max_error = 0.0;
//	for(int i=0;i<161604;i++) {
//		rel_error_sum += abs(heightData_cpu[i]-heightData_gpu[i]);
//		max_error = max(max_error,(double)abs(heightData_cpu[i]-heightData_gpu[i]));
//	}
//	cout << "average rel error " << rel_error_sum / 161604.0 << endl;
//	cout << "max error " << max_error << endl;
//
//	int mismatch_num = 0;
//	for(int i=0;i<161604;i++) {
//		if(abs(heightData_cpu[i] - heightData_gpu[i]) > 1) {
//			mismatch_num++;
//			//cout << "mismatch at " << i << ": " << heightData_cpu[i] << " vs " << heightData_gpu[i] << endl;
//		}
//	}
//	cout << "mismatch num: " << mismatch_num << endl;
//
//	// debug验证
//	vector<float>& debugData_cpu = planet_data_cpu.data.debugData;
//	vector<float>& debugData_gpu = planet_data_gpu.data.debugData;
//	if(debugData_cpu.size()!=161604||debugData_gpu.size()!=161604) {
//		cout << "no debug data" << endl;
//		return 0;
//	}
//
//	for(int i=0;i<10;i++) {
//		cout << debugData_cpu[i] << " " << debugData_gpu[i] << endl;
//	}
//	cout << endl;
//
//	double rel_error_sum_debug = 0.0;
//	double max_error_debug = 0.0;
//	for(int i=0;i<161604;i++) {
//		rel_error_sum_debug += fabs(debugData_cpu[i]-debugData_gpu[i]);
//		max_error_debug = max(max_error_debug,(double)fabs(debugData_cpu[i]-debugData_gpu[i]));
//	}
//	cout << "debug average rel error " << rel_error_sum_debug / 161604.0 << endl;
//	cout << "debug max error " << max_error_debug << endl;
//
//	int mismatch_num_debug = 0;
//	for(int i=0;i<161604;i++) {
//		if(abs(debugData_cpu[i] - debugData_gpu[i]) > 1e-2) {
//			mismatch_num_debug++;
//			//cout << "mismatch at " << i << ": " << debugData_cpu[i] << " vs " << debugData_gpu[i] << endl;
//		}
//	}
//	cout << "mismatch num: " << mismatch_num_debug << endl;
//	std::this_thread::sleep_for(std::chrono::seconds(1000));
//}
