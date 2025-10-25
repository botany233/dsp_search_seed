#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "json.hpp"

#include "check_seed.hpp"
#include "check_batch.hpp"

using namespace nlohmann;
using namespace std;
namespace py = pybind11;

string get_galaxy_data_c(int seed,int star_num)
{
	return get_galaxy_data(seed,star_num).dump(0);
}

vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,string& galaxy_str,string& galaxy_str_simple)
{
	bool check_no_veins = galaxy_str == galaxy_str_simple;
	json galaxy_condition = json::parse(galaxy_str);
	json galaxy_condition_simple = json::parse(galaxy_str_simple);
	return check_batch(start_seed,end_seed,start_star_num,end_star_num,galaxy_condition,galaxy_condition_simple,check_no_veins);
}

PYBIND11_MODULE(search_seed,m) {
	m.def("get_galaxy_data_c",&get_galaxy_data_c,py::arg("seed"),py::arg("star_num"));
	m.def("check_batch_c",&check_batch_c,py::arg("start_seed"),py::arg("end_seed"),py::arg("start_star_num"),py::arg("end_star_num"),py::arg("galaxy_condition"),py::arg("galaxy_condition_simple"));
}
