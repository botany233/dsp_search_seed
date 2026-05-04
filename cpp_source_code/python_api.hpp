#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

#include "data_struct.hpp"

using namespace std;
namespace py = pybind11;

int get_condition_level(const GalaxyCondition& galaxy_condition,bool quick);
void do_init();
GalaxyData get_galaxy_data_c(SeedStruct seed,bool quick);
vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,uint8_t resource_index,
	const GalaxyCondition& galaxy_condition,int check_level);
vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,uint8_t resource_index,
	const py::dict& galaxy_condition_dict,bool quick);
