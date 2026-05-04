#pragma once
#include "data_struct.hpp"
#include "astro_class.hpp"

bool check_galaxy_level_1(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
bool check_galaxy_level_2(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
bool check_galaxy_level_3(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
bool check_galaxy_level_4(GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
int get_need_generate_planet_index(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition);
void tag_need_veins_galaxy(GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition);