#pragma once
#include "json.hpp"

using namespace nlohmann;
using namespace std;

vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,json& galaxy_condition,json& galaxy_condition_simple,bool check_no_veins);