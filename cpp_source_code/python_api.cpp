#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include "json.hpp"

#include "check_seed.hpp"
#include "check_batch.hpp"
#include "data_struct.hpp"
#include "PlanetRawData.hpp"
#include "RandomTable.hpp"

using namespace nlohmann;
using namespace std;
namespace py = pybind11;

void do_init()
{
	static bool is_init = false;
	if(is_init)
		return;
	is_init = true;
	PlanetRawData::ReadData();
	RandomTable::GenerateSphericNormal();
};

bool is_need_veins(const json& galaxy_condition)
{
	if(galaxy_condition.contains("veins") || galaxy_condition.contains("veins_point"))
		return true;
	if(galaxy_condition.contains("stars")) {
		for(const json& star_condition: galaxy_condition["stars"]) {
			if(star_condition.contains("veins") || star_condition.contains("veins_point"))
				return true;
			if(star_condition.contains("planets")) {
				for(const json& planet_condition: star_condition["planets"]) {
					if(planet_condition.contains("veins") || planet_condition.contains("veins_point"))
						return true;
				}
			}
		}
	}
	if(galaxy_condition.contains("planets")) {
		for(const json& planet_condition: galaxy_condition["planets"]) {
			if(planet_condition.contains("veins") || planet_condition.contains("veins_point"))
				return true;
		}
	}
	return false;
}

uint16_t update_veins(const vector<int>& veins) {
	uint16_t need_veins = 0;
	for(int i=0;i<14;i++) {
		need_veins |= (veins[i] > 0) << i;
	}
	return need_veins;
}

void update_planet_condition(json& planet_condition) {
	uint16_t need_veins = 0;
	if(planet_condition.contains("veins")) {
		need_veins |= update_veins(planet_condition["veins"]);
	}
	if(planet_condition.contains("veins_point")) {
		need_veins |= update_veins(planet_condition["veins_point"]);
	}
	if(need_veins)
		planet_condition["need_veins"] = need_veins;
	return;
}

void update_star_condition(json& star_condition) {
	uint16_t need_veins = 0;
	if(star_condition.contains("veins")) {
		need_veins |= update_veins(star_condition["veins"]);
	}
	if(star_condition.contains("veins_point")) {
		need_veins |= update_veins(star_condition["veins_point"]);
	}
	if(need_veins)
		star_condition["need_veins"] = need_veins;
	if(star_condition.contains("planets")) {
		for(json& planet_condition: star_condition["planets"]) {
			update_planet_condition(planet_condition);
		}
	}
	return;
}

void update_galaxy_condition(json& galaxy_condition) {
	uint16_t need_veins = 0;
	if(galaxy_condition.contains("veins")) {
		need_veins |= update_veins(galaxy_condition["veins"]);
	}
	if(galaxy_condition.contains("veins_point")) {
		need_veins |= update_veins(galaxy_condition["veins_point"]);
	}
	if(need_veins)
		galaxy_condition["need_veins"] = need_veins;
	if(galaxy_condition.contains("stars")) {
		for(json& star_condition: galaxy_condition["stars"]) {
			update_star_condition(star_condition);
		}
	}
	if(galaxy_condition.contains("planets")) {
		for(json& planet_condition: galaxy_condition["planets"]) {
			update_planet_condition(planet_condition);
		}
	}
	return;
}

GalaxyStruct get_galaxy_data_c(int seed,int star_num)
{
	do_init();
	return get_galaxy_data(seed,star_num);
}

vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,const string& galaxy_str,bool quick_check)
{
	do_init();
	json galaxy_condition = json::parse(galaxy_str);
	update_galaxy_condition(galaxy_condition);
	int check_level = 3;
	if(quick_check)
		check_level = 2;
	if(!is_need_veins(galaxy_condition))
		check_level = 1;
	return check_batch(start_seed,end_seed,start_star_num,end_star_num,galaxy_condition,check_level);
}

vector<string> check_precise_c(vector<int>& seed_vector,vector<int>& star_num_vector,const string& galaxy_str,bool quick_check)
{
	do_init();
	json galaxy_condition = json::parse(galaxy_str);
	update_galaxy_condition(galaxy_condition);
	int check_level = 3;
	if(quick_check)
		check_level = 2;
	if(!is_need_veins(galaxy_condition))
		check_level = 1;
	return check_precise(seed_vector,star_num_vector,galaxy_condition,check_level);
}

PYBIND11_MODULE(search_seed,m) {
	py::class_<PlanetStruct>(m,"PlanetData")
		.def(py::init<>())
		.def_readwrite("name",&PlanetStruct::name)
		.def_readwrite("type",&PlanetStruct::type)
		.def_readwrite("singularity",&PlanetStruct::singularity)
		.def_readwrite("seed",&PlanetStruct::seed)
		.def_readwrite("lumino",&PlanetStruct::lumino)
		.def_readwrite("wind",&PlanetStruct::wind)
		.def_readwrite("radius",&PlanetStruct::radius)
		.def_readwrite("liquid",&PlanetStruct::liquid)
		.def_readwrite("is_gas",&PlanetStruct::is_gas)
		.def_readwrite("is_in_dsp",&PlanetStruct::is_in_dsp)
		.def_readwrite("is_on_dsp",&PlanetStruct::is_on_dsp)
		.def_readwrite("veins",&PlanetStruct::veins)
		.def_readwrite("veins_point",&PlanetStruct::veins_point)
		.def_readwrite("gas_veins",&PlanetStruct::gas_veins);
	py::class_<StarStruct>(m,"StarData")
		.def(py::init<>())
		.def_readwrite("type",&StarStruct::type)
		.def_readwrite("name",&StarStruct::name)
		.def_readwrite("seed",&StarStruct::seed)
		.def_readwrite("distance",&StarStruct::distance)
		.def_readwrite("dyson_lumino",&StarStruct::dyson_lumino)
		.def_readwrite("dyson_radius",&StarStruct::dyson_radius)
		.def_readwrite("planets",&StarStruct::planets)
		.def_readwrite("veins",&StarStruct::veins)
		.def_readwrite("veins_point",&StarStruct::veins_point)
		.def_readwrite("gas_veins",&StarStruct::gas_veins)
		.def_readwrite("liquid",&StarStruct::liquid);
	py::class_<GalaxyStruct>(m,"GalaxyData")
		.def(py::init<>())
		.def_readwrite("seed",&GalaxyStruct::seed)
		.def_readwrite("star_num",&GalaxyStruct::star_num)
		.def_readwrite("stars",&GalaxyStruct::stars)
		.def_readwrite("veins",&GalaxyStruct::veins)
		.def_readwrite("veins_point",&GalaxyStruct::veins_point)
		.def_readwrite("gas_veins",&GalaxyStruct::gas_veins)
		.def_readwrite("planet_type_nums",&GalaxyStruct::planet_type_nums)
		.def_readwrite("star_type_nums",&GalaxyStruct::star_type_nums)
		.def_readwrite("liquid",&GalaxyStruct::liquid);
	m.def("get_galaxy_data_c",&get_galaxy_data_c,py::arg("seed"),py::arg("star_num"));
	m.def("check_batch_c",&check_batch_c,py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),py::arg("end_star_num"),py::arg("galaxy_condition"),py::arg("quick_check"));
	m.def("check_precise_c",&check_precise_c,py::arg("seed_vector"),py::arg("star_num_vector"),py::arg("galaxy_condition"),py::arg("quick_check"));
}
