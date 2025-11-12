#pragma once
#include "json.hpp"
#include "data_struct.hpp"

using namespace nlohmann;

bool check_galaxy(GalaxyStructSimple& galaxy_data,const json& galaxy_condition,const bool update_flag);
bool check_galaxy_quick(const GalaxyStructSimple& galaxy_data,const json& galaxy_condition);