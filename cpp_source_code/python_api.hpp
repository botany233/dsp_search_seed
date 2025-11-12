#pragma once
#include <vector>

#include "data_struct.hpp"

using namespace std;

void do_init();
GalaxyStruct get_galaxy_data_c(int seed,int star_num);
vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,const string& galaxy_str,bool quick_check);
vector<string> check_precise_c(vector<int>& seed_vector,vector<int>& star_num_vector,const string& galaxy_str,bool quick_check);