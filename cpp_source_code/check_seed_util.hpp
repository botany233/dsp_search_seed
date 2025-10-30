#pragma once
#include "json.hpp"
#include "data_struct.hpp"

using namespace nlohmann;

bool check_galaxy(const GalaxyStructSimple& galaxy_data,const json& galaxy_condition);