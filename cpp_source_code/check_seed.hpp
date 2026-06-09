#pragma once
#include "data_struct.hpp"

GalaxyData get_galaxy_data(const SeedStruct& seed,int level);
bool check_seed(const SeedStruct& seed,const GalaxyCondition& galaxy_condition,int check_level);