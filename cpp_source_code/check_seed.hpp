#pragma once
#include "astro_class.hpp"
#include "data_struct.hpp"

GalaxyData get_galaxy_data(const SeedStruct& seed,bool quick);
GalaxyData get_galaxy_data_fast(const SeedStruct& seed,bool quick,int gen_planet_num=-1,unsigned short vein_mask=0x3FFF);
bool check_seed(const SeedStruct& seed,const GalaxyCondition& galaxy_condition,int check_level);