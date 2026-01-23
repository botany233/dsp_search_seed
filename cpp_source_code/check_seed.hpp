#pragma once
#include "data_struct.hpp"

GalaxyData get_galaxy_data(int seed,int star_num,bool quick);
bool check_seed_level_1(int seed,int star_num,const GalaxyCondition& galaxy_condition,int check_level);
