#pragma once

#include "check_seed.hpp"
#include "check_batch.hpp"

using namespace std;

vector<string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,string& galaxy_str,string& galaxy_str_simple);
vector<string> check_precise_c(vector<int>& seed_vector,vector<int>& star_num_vector,string& galaxy_str,string& galaxy_str_simple);