#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

#include "data_struct.hpp"

using namespace std;
namespace py = pybind11;

void do_init();
bool set_device_id_c(int device_id);
int get_device_id_c();
void set_local_size_c(int local_size);
int get_local_size_c();
vector<string> get_device_info_c();
bool get_support_double_c();
GalaxyStruct get_galaxy_data_c(int seed,int star_num,bool quick);
vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,const py::dict& galaxy_condition_dict,bool quick);
vector<string> check_precise_c(const vector<int>& seed_vector,const vector<int>& star_num_vector,const py::dict& galaxy_condition_dict,bool quick);