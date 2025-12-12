#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>

#include "check_seed.hpp"
#include "data_struct.hpp"
#include "PlanetRawData.hpp"
#include "RandomTable.hpp"
#include "PlanetAlgorithm.hpp"
#include "condition_to_struct.hpp"

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

bool set_device_id_c(int device_id) {
	return OpenCLManager::init_device(device_id);
}

int get_device_id_c() {
	if(OpenCLManager::SUPPORT_GPU)
		return OpenCLManager::device_id;
	else
		return -1;
}

void set_local_size_c(int local_size) {
	OpenCLManager::set_local_size(local_size);
}

int get_local_size_c() {
	return OpenCLManager::local_size;
}

vector<string> get_device_info_c() {
	return OpenCLManager::devices_info;
}

bool get_support_double_c() {
	return OpenCLManager::SUPPORT_GPU && OpenCLManager::SUPPORT_DOUBLE;
}

bool is_need_veins(const GalaxyCondition& galaxy_condition)
{
	if(galaxy_condition.need_veins)
		return true;
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		if(star_condition.need_veins)
			return true;
		for(const PlanetCondition& planet_condition: star_condition.planets) {
			if(planet_condition.need_veins)
				return true;
		}
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		if(planet_condition.need_veins)
			return true;
	}
	return false;
}

vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,const GalaxyCondition& galaxy_condition,int check_level)
{
	vector<string> result;
	for(int seed = start_seed;seed < end_seed;seed++)
	{
		for(int star_num = start_star_num;star_num<end_star_num;star_num++)
		{
			if(check_seed_level_1(seed,star_num,galaxy_condition,check_level))
				result.push_back(to_string(seed) + ", " + to_string(star_num));
		}
	}
	return result;
}

vector<string> check_precise(const vector<int>& seed_vector,const vector<int>& star_num_vector,const GalaxyCondition& galaxy_condition,int check_level)
{
	vector<string> result;
	for(int i = 0; i<seed_vector.size(); i++)
	{
		int seed = seed_vector[i];
		int star_num = star_num_vector[i];
		if(check_seed_level_1(seed,star_num,galaxy_condition,check_level))
			result.push_back(to_string(seed) + ", " + to_string(star_num));
	}
	return result;
}

GalaxyStruct get_galaxy_data_c(int seed,int star_num,bool quick)
{
	return get_galaxy_data(seed,star_num,quick);
}

vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,const py::dict& galaxy_condition_dict,bool quick)
{
	GalaxyCondition galaxy_condition = galaxy_condition_to_struct(galaxy_condition_dict);
	int check_level = 3;
	if(quick)
		check_level = 2;
	if(!is_need_veins(galaxy_condition))
		check_level = 1;
	return check_batch(start_seed,end_seed,start_star_num,end_star_num,galaxy_condition,check_level);
}

vector<string> check_precise_c(const vector<int>& seed_vector,const vector<int>& star_num_vector,const py::dict& galaxy_condition_dict,bool quick)
{
	GalaxyCondition galaxy_condition = galaxy_condition_to_struct(galaxy_condition_dict);
	int check_level = 3;
	if(quick)
		check_level = 2;
	if(!is_need_veins(galaxy_condition))
		check_level = 1;
	return check_precise(seed_vector,star_num_vector,galaxy_condition,check_level);
}

PYBIND11_MODULE(search_seed,m) {
	py::class_<PlanetCondition>(m,"PlanetCondition")
		.def(py::init<>())
		.def_readwrite("satisfy_num",&PlanetCondition::satisfy_num)
		.def_readwrite("dsp_level",&PlanetCondition::dsp_level)
		.def_readwrite("type",&PlanetCondition::type)
		.def_readwrite("liquid",&PlanetCondition::liquid)
		.def_readwrite("singularity",&PlanetCondition::singularity)
		.def_readwrite("need_veins",&PlanetCondition::need_veins)
		.def_readwrite("veins_group",&PlanetCondition::veins_group)
		.def_readwrite("veins_point",&PlanetCondition::veins_point);
	py::class_<StarCondition>(m,"StarCondition")
		.def(py::init<>())
		.def_readwrite("satisfy_num",&StarCondition::satisfy_num)
		.def_readwrite("type",&StarCondition::type)
		.def_readwrite("distance",&StarCondition::distance)
		.def_readwrite("dyson_lumino",&StarCondition::dyson_lumino)
		.def_readwrite("need_veins",&StarCondition::need_veins)
		.def_readwrite("veins_group",&StarCondition::veins_group)
		.def_readwrite("veins_point",&StarCondition::veins_point)
		.def_readwrite("planets",&StarCondition::planets);
	py::class_<GalaxyCondition>(m,"GalaxyCondition")
		.def(py::init<>())
		.def_readwrite("need_veins",&GalaxyCondition::need_veins)
		.def_readwrite("veins_group",&GalaxyCondition::veins_group)
		.def_readwrite("veins_point",&GalaxyCondition::veins_point)
		.def_readwrite("stars",&GalaxyCondition::stars)
		.def_readwrite("planets",&GalaxyCondition::planets);
	py::class_<PlanetStruct>(m,"PlanetData")
		.def(py::init<>())
		.def_readwrite("name",&PlanetStruct::name)
		.def_readwrite("type",&PlanetStruct::type)
		.def_readwrite("type_id",&PlanetStruct::type_id)
		.def_readwrite("singularity",&PlanetStruct::singularity)
		.def_readwrite("singularity_str",&PlanetStruct::singularity_str)
		.def_readwrite("seed",&PlanetStruct::seed)
		.def_readwrite("lumino",&PlanetStruct::lumino)
		.def_readwrite("wind",&PlanetStruct::wind)
		.def_readwrite("radius",&PlanetStruct::radius)
		.def_readwrite("liquid",&PlanetStruct::liquid)
		.def_readwrite("is_gas",&PlanetStruct::is_gas)
		.def_readwrite("dsp_level",&PlanetStruct::dsp_level)
		.def_readwrite("veins_group",&PlanetStruct::veins_group)
		.def_readwrite("veins_point",&PlanetStruct::veins_point)
		.def_readwrite("gas_veins",&PlanetStruct::gas_veins);
	py::class_<StarStruct>(m,"StarData")
		.def(py::init<>())
		.def_readwrite("type",&StarStruct::type)
		.def_readwrite("type_id",&StarStruct::type_id)
		.def_readwrite("name",&StarStruct::name)
		.def_readwrite("seed",&StarStruct::seed)
		.def_readwrite("distance",&StarStruct::distance)
		.def_readwrite("dyson_lumino",&StarStruct::dyson_lumino)
		.def_readwrite("dyson_radius",&StarStruct::dyson_radius)
		.def_readwrite("planets",&StarStruct::planets)
		.def_readwrite("veins_group",&StarStruct::veins_group)
		.def_readwrite("veins_point",&StarStruct::veins_point)
		.def_readwrite("gas_veins",&StarStruct::gas_veins)
		.def_readwrite("liquid",&StarStruct::liquid);
	py::class_<GalaxyStruct>(m,"GalaxyData")
		.def(py::init<>())
		.def_readwrite("seed",&GalaxyStruct::seed)
		.def_readwrite("star_num",&GalaxyStruct::star_num)
		.def_readwrite("stars",&GalaxyStruct::stars)
		.def_readwrite("veins_group",&GalaxyStruct::veins_group)
		.def_readwrite("veins_point",&GalaxyStruct::veins_point)
		.def_readwrite("gas_veins",&GalaxyStruct::gas_veins)
		.def_readwrite("liquid",&GalaxyStruct::liquid);
	m.def("do_init_c",&do_init);
	m.def("set_device_id_c",&set_device_id_c,py::arg("device_id"));
	m.def("get_device_id_c",&get_device_id_c);
	m.def("set_local_size_c",&set_local_size_c,py::arg("local_size"));
	m.def("get_local_size_c",&get_local_size_c);
	m.def("get_device_info_c",&get_device_info_c);
	m.def("get_support_double_c",&get_support_double_c);
	m.def("galaxy_condition_to_struct",&galaxy_condition_to_struct,py::arg("galaxy_condition"));
	m.def("get_galaxy_data_c",&get_galaxy_data_c,py::arg("seed"),py::arg("star_num"),py::arg("quick"));
	m.def("check_batch_c",&check_batch_c,py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),py::arg("end_star_num"),py::arg("galaxy_condition"),py::arg("quick"));
	m.def("check_precise_c",&check_precise_c,py::arg("seed_vector"),py::arg("star_num_vector"),py::arg("galaxy_condition"),py::arg("quick"));
}
