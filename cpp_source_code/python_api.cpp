#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <variant>
#include <array>

#include "defines.hpp"
#include "check_batch.hpp"
#include "check_seed.hpp"
#include "data_struct.hpp"
#include "PlanetRawData.hpp"
#include "RandomTable.hpp"
#include "PlanetAlgorithm.hpp"
#include "condition_to_struct.hpp"
#include "gpu_benchmark.hpp"

using namespace std;
namespace py = pybind11;

void do_init()
{
	static bool is_init = false;
	if(is_init)
		return;
	is_init = true;
	PlanetRawData::ReadData();
	OpenCLManager::do_init();
	RandomTable::GenerateSphericNormal();
};

vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,uint8_t resource_index,
	const GalaxyCondition& galaxy_condition,int check_level)
{
	vector<string> result;
	for(int seed_id = start_seed;seed_id < end_seed;seed_id++)
	{
		for(int star_num = start_star_num;star_num<end_star_num;star_num++)
		{
			if(check_seed(SeedStruct(seed_id,star_num,resource_index),galaxy_condition,check_level))
				result.push_back(to_string(seed_id) + "," + to_string(star_num));
		}
	}
	return result;
}

static bool is_planet_need_veins(const PlanetCondition& planet_condition) {
	if(planet_condition.need_veins)
		return true;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		if(moon_condition.need_veins)
			return true;
	}
	return false;
}

static bool is_star_need_veins(const StarCondition& star_condition) {
	if(star_condition.need_veins)
		return true;
	for(const PlanetCondition& planet_condition: star_condition.planets) {
		if(is_planet_need_veins(planet_condition))
			return true;
	}
	return false;
}

static bool is_need_veins(const GalaxyCondition& galaxy_condition)
{
	if(galaxy_condition.need_veins)
		return true;
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		if(is_star_need_veins(star_condition))
			return true;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		if(is_planet_need_veins(planet_condition))
			return true;
	}
	for(const BondCondition& bond_condition: galaxy_condition.bonds) {
		if(holds_alternative<PlanetCondition>(bond_condition.con1)) {
			if(is_planet_need_veins(get<PlanetCondition>(bond_condition.con1)))
				return true;
		} else {
			if(is_star_need_veins(get<StarCondition>(bond_condition.con1)))
				return true;
		}
		if(holds_alternative<PlanetCondition>(bond_condition.con2)) {
			if(is_planet_need_veins(get<PlanetCondition>(bond_condition.con2)))
				return true;
		} else {
			if(is_star_need_veins(get<StarCondition>(bond_condition.con2)))
				return true;
		}
	}
	return false;
}

static bool is_need_planet(const GalaxyCondition& galaxy_condition)
{
	if(!galaxy_condition.bonds.empty())
		return true;
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		if(star_condition.planets.size() > 0)
			return true;
	}
	return galaxy_condition.planets.size() > 0;
}

int get_condition_level(const GalaxyCondition& galaxy_condition,bool quick) {
	if(is_need_veins(galaxy_condition)) {
		if(quick)
			return 3;
		else
			return 4;
	} else {
		if(is_need_planet(galaxy_condition))
			return 2;
		else
			return 1;
	}
}

vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,uint8_t resource_index,
	const py::dict& galaxy_condition_dict,bool quick)
{
	GalaxyCondition galaxy_condition = galaxy_condition_to_struct(galaxy_condition_dict);
	int check_level = get_condition_level(galaxy_condition,quick);
	return check_batch(start_seed,end_seed,start_star_num,end_star_num,resource_index,galaxy_condition,check_level);
}

PYBIND11_MODULE(search_seed,m) {
	py::class_<SeedStruct>(m,"Seed")
		.def(py::init<>())
		.def(py::init<int,int,uint8_t>())
		.def_readonly("seed_id",&SeedStruct::seed_id)
		.def_readonly("star_num",&SeedStruct::star_num)
		.def_readonly("resource_index",&SeedStruct::resource_index);
	py::class_<PlanetCondition>(m,"PlanetCondition")
		.def(py::init<>())
		.def_readonly("satisfy_num",&PlanetCondition::satisfy_num)
		.def_readonly("dsp_level",&PlanetCondition::dsp_level)
		.def_readonly("type",&PlanetCondition::type)
		.def_readonly("liquid",&PlanetCondition::liquid)
		.def_readonly("singularity",&PlanetCondition::singularity)
		.def_readonly("need_veins",&PlanetCondition::need_veins)
		.def_readonly("veins_point",&PlanetCondition::veins_point)
		.def_readonly("veins_amount",&PlanetCondition::veins_amount)
		.def_readonly("moons",&PlanetCondition::moons);;
	py::class_<StarCondition>(m,"StarCondition")
		.def(py::init<>())
		.def_readonly("satisfy_num",&StarCondition::satisfy_num)
		.def_readonly("type",&StarCondition::type)
		.def_readonly("distance",&StarCondition::distance)
		.def_readonly("dyson_lumino",&StarCondition::dyson_lumino)
		.def_readonly("need_veins",&StarCondition::need_veins)
		.def_readonly("veins_point",&StarCondition::veins_point)
		.def_readonly("veins_amount",&StarCondition::veins_amount)
		.def_readonly("planets",&StarCondition::planets);
	py::class_<BondCondition>(m,"BondCondition")
		.def(py::init<>())
		.def_readonly("distance",&BondCondition::distance)
		.def_readonly("satisfy_num",&BondCondition::satisfy_num)
		.def_property_readonly("con1",[](const BondCondition& self) { return self.con1; })
		.def_property_readonly("con2",[](const BondCondition& self) { return self.con2; });
	py::class_<GalaxyCondition>(m,"GalaxyCondition")
		.def(py::init<>())
		.def_readonly("need_veins",&GalaxyCondition::need_veins)
		.def_readonly("veins_point",&GalaxyCondition::veins_point)
		.def_readonly("veins_amount",&GalaxyCondition::veins_amount)
		.def_readonly("stars",&GalaxyCondition::stars)
		.def_readonly("planets",&GalaxyCondition::planets)
		.def_readonly("bonds", &GalaxyCondition::bonds);
	py::class_<PlanetData>(m,"PlanetData")
		.def(py::init<>())
		.def_readonly("name",&PlanetData::name)
		.def_readonly("type",&PlanetData::type)
		.def_readonly("type_id",&PlanetData::type_id)
		.def_readonly("singularity",&PlanetData::singularity)
		.def_readonly("singularity_str",&PlanetData::singularity_str)
		.def_readonly("pos_m",&PlanetData::pos_m)
		.def_readonly("pos_ly",&PlanetData::pos_ly)
		.def_readonly("seed",&PlanetData::seed)
		.def_readonly("lumino",&PlanetData::lumino)
		.def_readonly("wind",&PlanetData::wind)
		.def_readonly("radius",&PlanetData::radius)
		.def_readonly("liquid",&PlanetData::liquid)
		.def_readonly("is_gas",&PlanetData::is_gas)
		.def_readonly("dsp_level",&PlanetData::dsp_level)
		.def_readonly("raw_dsp_degree",&PlanetData::raw_dsp_degree)
		.def_readonly("enhance_dsp_degree",&PlanetData::enhance_dsp_degree)
		.def_readonly("obliquity",&PlanetData::obliquity)
		.def_readonly("veins_point",&PlanetData::veins_point)
		.def_readonly("veins_amount",&PlanetData::veins_amount)
		.def_readonly("gas_veins",&PlanetData::gas_veins)
		.def_readonly("moons",&PlanetData::moons);
	py::class_<StarData>(m,"StarData")
		.def(py::init<>())
		.def_readonly("type",&StarData::type)
		.def_readonly("type_id",&StarData::type_id)
		.def_readonly("name",&StarData::name)
		.def_readonly("seed",&StarData::seed)
		.def_readonly("dyson_lumino",&StarData::dyson_lumino)
		.def_readonly("dyson_radius",&StarData::dyson_radius)
		.def_readonly("distance",&StarData::distance)
		.def_readonly("pos_ly",&StarData::pos_ly)
		.def_readonly("pos_m",&StarData::pos_m)
		.def_readonly("planets",&StarData::planets)
		.def_readonly("veins_point",&StarData::veins_point)
		.def_readonly("veins_amount",&StarData::veins_amount)
		.def_readonly("gas_veins",&StarData::gas_veins)
		.def_readonly("liquid",&StarData::liquid);
	py::class_<GalaxyData>(m,"GalaxyData")
		.def(py::init<>())
		.def_readonly("seed_id",&GalaxyData::seed_id)
		.def_readonly("star_num",&GalaxyData::star_num)
		.def_readonly("resource_index",&GalaxyData::resource_index)
		.def_readonly("resource_rate",&GalaxyData::resource_rate)
		.def_readonly("stars",&GalaxyData::stars)
		.def_readonly("veins_point",&GalaxyData::veins_point)
		.def_readonly("veins_amount",&GalaxyData::veins_amount)
		.def_readonly("gas_veins",&GalaxyData::gas_veins)
		.def_readonly("liquid",&GalaxyData::liquid);
	py::class_<SeedManager>(m,"SeedManager")
		.def(py::init<>())
		.def("add_seed",&SeedManager::add_seed)
		.def("del_seed",&SeedManager::del_seed)
		.def("clear",&SeedManager::clear)
		.def("reset_index",&SeedManager::reset_index)
		.def("get_seeds",&SeedManager::get_seeds)
		.def("get_seeds_count",&SeedManager::get_seeds_count);
	py::class_<GetDataManager>(m,"GetDataManager")
		.def(py::init<int,bool,int>())
		.def("add_task",&GetDataManager::add_task)
		.def("shutdown",&GetDataManager::shutdown)
		.def("get_results",&GetDataManager::get_results);
	py::class_<CheckPreciseManager>(m,"CheckPreciseManager")
		.def(py::init<SeedManager&,uint8_t,const py::dict &,bool,int>())
		.def("run",&CheckPreciseManager::run)
		.def("start_wait",&CheckPreciseManager::start_wait)
		.def("end_wait",&CheckPreciseManager::end_wait)
		.def("is_running",&CheckPreciseManager::is_running)
		.def("shutdown",&CheckPreciseManager::shutdown)
		.def("get_task_num",&CheckPreciseManager::get_task_num)
		.def("get_task_progress",&CheckPreciseManager::get_task_progress)
		.def("get_result_num",&CheckPreciseManager::get_result_num)
		.def("get_last_result",&CheckPreciseManager::get_last_result)
		.def("get_results",&CheckPreciseManager::get_results);
	py::class_<CheckBatchManager>(m,"CheckBatchManager")
		.def(py::init<int,int,int,int,uint8_t,const py::dict&,bool,int>())
		.def("run", &CheckBatchManager::run)
		.def("start_wait",&CheckBatchManager::start_wait)
		.def("end_wait",&CheckBatchManager::end_wait)
		.def("is_running",&CheckBatchManager::is_running)
		.def("shutdown", &CheckBatchManager::shutdown)
		.def("get_task_num",&CheckBatchManager::get_task_num)
		.def("get_task_progress",&CheckBatchManager::get_task_progress)
		.def("get_result_num",&CheckBatchManager::get_result_num)
		.def("get_last_result", &CheckBatchManager::get_last_result)
		.def("get_results", &CheckBatchManager::get_results);
	py::class_<GPUBenchmark>(m,"GPUBenchmark")
		.def(py::init<int>())
		.def("run",&GPUBenchmark::run)
		.def("shutdown",&GPUBenchmark::shutdown)
		.def("reset",&GPUBenchmark::reset)
		.def("get_speed",&GPUBenchmark::get_speed);
	m.def("do_init_c",&do_init);
	m.def("set_device_id_c",&OpenCLManager::set_device_id,py::arg("device_id"));
	m.def("get_device_id_c",&OpenCLManager::get_device_id);
	m.def("set_local_size_c",&OpenCLManager::set_local_size,py::arg("local_size"));
	m.def("get_local_size_c",&OpenCLManager::get_local_size);
	m.def("get_device_info_c",&OpenCLManager::get_devices_info);
	m.def("get_support_double_c",&OpenCLManager::get_support_double);
	m.def("set_gpu_max_worker_c",&OpenCLManager::set_max_worker,py::arg("max_worker"));
	m.def("get_gpu_max_worker_c",&OpenCLManager::get_max_worker);
	m.def("galaxy_condition_to_struct",&galaxy_condition_to_struct,py::arg("galaxy_condition"));
	m.def("get_galaxy_data_c",&get_galaxy_data,py::arg("seed"),py::arg("quick"));
	m.def("get_galaxy_data_fast_c",&get_galaxy_data_fast,py::arg("seed"),py::arg("quick"),py::arg("gen_planet_num")=-1,py::arg("vein_mask")=0x3FFF);
	m.def("check_batch_c",&check_batch_c,py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),py::arg("end_star_num"),py::arg("resource_index"),py::arg("galaxy_condition"),py::arg("quick"));
}
