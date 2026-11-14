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
#include "get_galaxy_data.hpp"
#include "check_seed.hpp"
#include "data_struct.hpp"
#include "src/RandomTable.hpp"
#include "PlanetAlgorithm.hpp"
#include "condition_struct.hpp"
#include "gpu_benchmark.hpp"

namespace py = pybind11;

void do_init()
{
	static bool is_init = false;
	if(is_init)
		return;
	is_init = true;
	PlanetAlgorithm::do_init();
	OpenCLManager::do_init();
	RandomTable::GenerateSphericNormal();
};

std::vector<std::string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,uint8_t resource_index,
	const ConditionManager& galaxy_condition,bool quick)
{
	std::vector<std::string> result;
	WorkSpace workspace;
	workspace.galaxy.max_position = start_seed < end_seed && end_star_num - start_star_num > 1 ?
		static_cast<std::uint8_t>(end_star_num - 1) : 0;
	for(int seed_id = start_seed;seed_id < end_seed;seed_id++)
	{
		for(int star_num = start_star_num;star_num<end_star_num;star_num++)
		{
			if(check_seed(SeedStruct(seed_id,star_num,resource_index),galaxy_condition,quick,workspace))
				result.push_back(std::to_string(seed_id) + "," + std::to_string(star_num));
		}
	}
	return result;
}

PYBIND11_MODULE(search_seed,m) {
	py::class_<SeedStruct>(m,"Seed")
		.def(py::init<>())
		.def(py::init<int,int,uint8_t>(),py::arg("seed_id"),py::arg("star_num"),py::arg("resource_index"))
		.def_readonly("seed_id",&SeedStruct::seed_id)
		.def_readonly("star_num",&SeedStruct::star_num)
		.def_readonly("resource_index",&SeedStruct::resource_index);
	py::class_<ConditionManager>(m,"ConditionManager")
		.def(py::init<const py::dict&>(),py::arg("condition"))
		.def(py::init<const std::string&>(),py::arg("condition_json"));
	py::class_<PlanetData>(m,"PlanetData")
		.def(py::init<>())
		.def_readonly("star_index",&PlanetData::star_index)
		.def_readonly("planet_index",&PlanetData::planet_index)
		.def_readonly("name_enus",&PlanetData::name_enus)
		.def_readonly("name_zhcn",&PlanetData::name_zhcn)
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
		.def_readonly("land_percent",&PlanetData::land_percent)
		.def_readonly("veins_point",&PlanetData::veins_point)
		.def_readonly("veins_amount",&PlanetData::veins_amount)
		.def_readonly("gas_veins",&PlanetData::gas_veins)
		.def_readonly("moons",&PlanetData::moons);
	py::class_<StarData>(m,"StarData")
		.def(py::init<>())
		.def_readonly("star_index",&StarData::star_index)
		.def_readonly("type",&StarData::type)
		.def_readonly("type_id",&StarData::type_id)
		.def_readonly("name_enus",&StarData::name_enus)
		.def_readonly("name_zhcn",&StarData::name_zhcn)
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
	py::class_<GalaxyDataSimple>(m,"GalaxyDataSimple")
		.def(py::init<>())
		.def_readonly("seed_id",&GalaxyDataSimple::seed_id)
		.def_readonly("star_num",&GalaxyDataSimple::star_num)
		.def_readonly("resource_index",&GalaxyDataSimple::resource_index)
		.def_readonly("star_type_num",&GalaxyDataSimple::star_type_num)
		.def_readonly("planet_type_num",&GalaxyDataSimple::planet_type_num)
		.def_readonly("veins_point",&GalaxyDataSimple::veins_point)
		.def_readonly("veins_amount",&GalaxyDataSimple::veins_amount);
	py::class_<SeedManager>(m,"SeedManager")
		.def(py::init<>())
		.def("add_seed",&SeedManager::add_seed,py::arg("seed_id"),py::arg("star_num"))
		.def("del_seed",&SeedManager::del_seed,py::arg("seed_id"),py::arg("star_num"))
		.def("clear",&SeedManager::clear)
		.def("reset_index",&SeedManager::reset_index)
		.def("get_seeds",&SeedManager::get_seeds,py::arg("batch_num"),py::arg("resource_index"))
		.def("get_seeds_count",&SeedManager::get_seeds_count);
	py::class_<GetDataQueue>(m,"GetDataQueue")
		.def(py::init<int>(),py::arg("max_cache"))
		.def("add_task",&GetDataQueue::add_task,py::arg("seed"),py::arg("thread_num"))
		.def("shutdown",&GetDataQueue::shutdown)
		.def("get_results",&GetDataQueue::get_results);
	py::class_<GetDataManager>(m,"GetDataManager")
		.def(py::init<int,bool,int>(),py::arg("max_thread"),py::arg("quick"),py::arg("max_cache"))
		.def("add_task",&GetDataManager::add_task,py::arg("seed"))
		.def("shutdown",&GetDataManager::shutdown)
		.def("get_results",&GetDataManager::get_results);
	py::class_<GetDataManagerFast>(m,"GetDataManagerFast")
		.def(py::init<int,bool,int,int,unsigned short>(),py::arg("max_thread"),py::arg("quick"),
			py::arg("max_cache")=1024,py::arg("gen_planet_num")=-1,py::arg("vein_mask")=0x3FFF)
		.def("add_task",&GetDataManagerFast::add_task,py::arg("seed"))
		.def("shutdown",&GetDataManagerFast::shutdown)
		.def("get_results",&GetDataManagerFast::get_results);
	py::class_<CheckPreciseManager>(m,"CheckPreciseManager")
		.def(py::init<SeedManager&,uint8_t,const py::dict &,bool,int>(),py::arg("seed_manager"),
			py::arg("resource_index"),py::arg("galaxy_condition"),py::arg("quick"),py::arg("max_thread"),py::keep_alive<1,2>())
		.def(py::init<SeedManager&,uint8_t,const ConditionManager&,bool,int>(),py::arg("seed_manager"),
			py::arg("resource_index"),py::arg("galaxy_condition"),py::arg("quick"),py::arg("max_thread"),py::keep_alive<1,2>())
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
		.def(py::init<int,int,int,int,uint8_t,const py::dict&,bool,int>(),py::arg("start_seed"),py::arg("end_seed"),
			py::arg("start_star_num"),py::arg("end_star_num"),py::arg("resource_index"),
			py::arg("galaxy_condition"),py::arg("quick"),py::arg("max_thread"))
		.def(py::init<int,int,int,int,uint8_t,const ConditionManager&,bool,int>(),py::arg("start_seed"),py::arg("end_seed"),
			py::arg("start_star_num"),py::arg("end_star_num"),py::arg("resource_index"),
			py::arg("galaxy_condition"),py::arg("quick"),py::arg("max_thread"))
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
		.def(py::init<int,bool>(),py::arg("max_thread"),py::arg("heavy"))
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
	m.def("set_gpu_max_worker_c",&OpenCLManager::set_max_worker,py::arg("max_worker"));
	m.def("get_gpu_max_worker_c",&OpenCLManager::get_max_worker);
	m.def("check_seed_c",py::overload_cast<const SeedStruct&,const ConditionManager&,bool>(&check_seed),
		py::arg("seed"),py::arg("galaxy_condition"),py::arg("quick"));
	m.def("check_batch_c",&check_batch_c,py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),
		py::arg("end_star_num"),py::arg("resource_index"),py::arg("galaxy_condition"),py::arg("quick"));
	m.def("get_galaxy_data_c",&get_galaxy_data,py::arg("seed"),py::arg("quick"));
	m.def("get_galaxy_data_fast_c",&get_galaxy_data_fast,py::arg("seed"),py::arg("quick"),py::arg("gen_planet_num")=-1,py::arg("vein_mask")=0x3FFF);
	m.def("check_batch_c",[](int start_seed,int end_seed,int start_star_num,int end_star_num,std::uint8_t resource_index,
		const py::dict& condition,bool quick) {
		return check_batch_c(start_seed,end_seed,start_star_num,end_star_num,resource_index,ConditionManager(condition),quick);
	},py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),py::arg("end_star_num"),
		py::arg("resource_index"),py::arg("galaxy_condition"),py::arg("quick"));
}
