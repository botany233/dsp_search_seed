#pragma once
#include "data_struct.hpp"

GalaxyData get_galaxy_data_para(const SeedStruct& seed,int thread_num);
GalaxyData get_galaxy_data(const SeedStruct& seed,bool quick);
GalaxyData get_galaxy_data_fast(const SeedStruct& seed,bool quick,int gen_planet_num=-1,unsigned short vein_mask=0x3FFF);
