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

using namespace std;
using namespace chrono;

int main() {
	system("chcp 65001>nul");
	do_init();
	set_device_id_c(0);
	GalaxyData galaxy_data = get_galaxy_data(5121007,64,false);
	for(const StarData& star_data: galaxy_data.stars) {
		cout << "star " << star_data.type << " " << star_data.name << endl;
		for(const PlanetData& planet_data: star_data.planets) {
			cout << "    planet " << planet_data.type << " " << planet_data.name << endl;
		}
	}
}

//// 测试gpu和cpu的星系生成效率
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

//性能测试
//int main(int argc, char* argv[])
//{
//    system("chcp 65001>nul");
//	
//	StarCondition star_condition1 = StarCondition();
//	star_condition1.satisfy_num = 3;
//	star_condition1.type = 3;
//	GalaxyCondition galaxy_condition = GalaxyCondition();
//	galaxy_condition.stars.push_back(star_condition1);
//
//	do_init();
//
//	//bool result = check_seed_level_1(497,64,galaxy_condition,3);
//	//cout << result << endl;
//
//	auto start = high_resolution_clock::now();
//	vector<string> results = check_batch(0,20000,64,65,galaxy_condition,1);
//	auto end = high_resolution_clock::now();
//	auto duration = duration_cast<milliseconds>(end - start);
//	cout << "代码运行时间: " << duration.count() << " 毫秒，找到"<< results.size() << "个种子" << endl;
//
//	start = high_resolution_clock::now();
//	int result_num = 0;
//	for(int i=0;i<20000;i++) {
//		if(check_seed_new(i,64,galaxy_condition))
//			result_num++;
//	}
//	end = high_resolution_clock::now();
//	duration = duration_cast<milliseconds>(end - start);
//	cout << "代码运行时间: " << duration.count() << " 毫秒，找到"<< result_num << "个种子" << endl;
//
//	//auto start = high_resolution_clock::now();
//	//CheckBatchManager check_batch_manager = CheckBatchManager(0,50000,32,65,galaxy_condition,true,20);
//	//check_batch_manager.run();
//	////int last_result_num = 0;
//	////SeedStruct last_valid_seed = SeedStruct(-1,-1);
//	//while(check_batch_manager.is_running()) {
//	//	cout << "进度: " << check_batch_manager.get_task_progress() << " / " << check_batch_manager.get_task_num() << "\r";
//	//	//int current_result_num = check_batch_manager.get_result_num();
//	//	//last_valid_seed = check_batch_manager.get_last_result();
//	//	//if(current_result_num > last_result_num) {
//	//	//	//cout << "找到新种子: (" << last_valid_seed.seed_id << ", " << last_valid_seed.star_num << ")" << endl;
//	//	//	last_result_num = current_result_num;
//	//	//}
//	//	this_thread::sleep_for(chrono::milliseconds(50));
//	//}
//	//vector<SeedStruct> results = check_batch_manager.get_results();
//	//auto end = high_resolution_clock::now();
//	//auto duration = duration_cast<milliseconds>(end - start);
//	//cout << "代码运行时间: " << duration.count() << " 毫秒，找到"<< results.size() << "个种子" << endl;
//	////for(SeedStruct& result: results)
//	////	cout << "(" << result.seed_id << ", " << result.star_num << "), ";
//	////cout << endl;
//    return 0;
//}
