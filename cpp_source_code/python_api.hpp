#pragma once

#include "check_seed.hpp"
#include "check_batch.hpp"

using namespace std;

string get_galaxy_data_c(int seed,int star_num);
vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,string& galaxy_str,string& galaxy_str_simple);