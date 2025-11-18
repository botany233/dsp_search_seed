#pragma once
#include <vector>

#include "data_struct.hpp"

using namespace std;

void do_init();
GalaxyStruct get_galaxy_data_c(int seed,int star_num,bool quick);
vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,const py::dict& galaxy_condition_dict,bool quick);
vector<string> check_precise_c(const vector<int>& seed_vector,const vector<int>& star_num_vector,const py::dict& galaxy_condition_dict,bool quick);