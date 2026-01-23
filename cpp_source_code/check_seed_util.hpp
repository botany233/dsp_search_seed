#pragma once
#include "data_struct.hpp"

bool check_galaxy(const GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
bool check_galaxy_quick(const GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
void get_galaxy_condition_struct(const GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition,GalaxyConditionSimple& galaxy_condition_simple);
GalaxyConditionSimple init_galaxy_condition_struct(const GalaxyCondition& galaxy_condition);