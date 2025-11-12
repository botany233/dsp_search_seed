#pragma once
#include "json.hpp"

using namespace nlohmann;
using namespace std;

vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,const json& galaxy_condition,int check_level);
vector<string> check_precise(vector<int>& seed_vector,vector<int>& star_num_vector,const json& galaxy_condition,int check_level);