#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include "json.hpp"

#include "check_seed.hpp"
#include "check_batch.hpp"
#include "data_struct.hpp"

using namespace nlohmann;
using namespace std;
namespace py = pybind11;

vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,string& galaxy_str,string& galaxy_str_simple)
{
	bool check_no_veins = galaxy_str == galaxy_str_simple;
	json galaxy_condition = json::parse(galaxy_str);
	json galaxy_condition_simple = json::parse(galaxy_str_simple);
	return check_batch(start_seed,end_seed,start_star_num,end_star_num,galaxy_condition,galaxy_condition_simple,check_no_veins);
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
		.def_readwrite("gas_veins",&StarStruct::gas_veins)
		.def_readwrite("liquid",&StarStruct::liquid);
	py::class_<GalaxyStruct>(m,"GalaxyData")
		.def(py::init<>())
		.def_readwrite("seed",&GalaxyStruct::seed)
		.def_readwrite("star_num",&GalaxyStruct::star_num)
		.def_readwrite("stars",&GalaxyStruct::stars)
		.def_readwrite("veins",&GalaxyStruct::veins)
		.def_readwrite("gas_veins",&GalaxyStruct::gas_veins)
		.def_readwrite("planet_type_nums",&GalaxyStruct::planet_type_nums)
		.def_readwrite("star_type_nums",&GalaxyStruct::star_type_nums)
		.def_readwrite("liquid",&GalaxyStruct::liquid);
	m.def("get_galaxy_data_c",&get_galaxy_data,py::arg("seed"),py::arg("star_num"));
	m.def("check_batch_c",&check_batch_c,py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),py::arg("end_star_num"),py::arg("galaxy_condition"),py::arg("galaxy_condition_simple"));
}
