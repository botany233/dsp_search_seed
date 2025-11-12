#pragma once
#include "json.hpp"
#include "data_struct.hpp"

using namespace nlohmann;

GalaxyStruct get_galaxy_data(int seed,int star_num);
bool check_seed_level_1(int seed,int star_num,const json& galaxy_condition,int check_level);
