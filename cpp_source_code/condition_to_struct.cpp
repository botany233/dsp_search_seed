#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "data_struct.hpp"

using namespace std;
namespace py = pybind11;

MoonCondition moon_condition_to_struct(py::dict moon_condition) {
	MoonCondition new_moon_condition = MoonCondition();
	new_moon_condition.satisfy_num = moon_condition["satisfy_num"].cast<uint8_t>();
	new_moon_condition.dsp_level = moon_condition["dsp_level"].cast<uint8_t>();
	new_moon_condition.type = moon_condition["type"].cast<uint8_t>();
	new_moon_condition.liquid = moon_condition["liquid"].cast<uint8_t>();
	new_moon_condition.singularity = moon_condition["singularity"].cast<uint8_t>();
	new_moon_condition.need_veins = moon_condition["need_veins"].cast<uint16_t>();
	new_moon_condition.veins_group = moon_condition["veins_group"].cast<vector<int>>();
	new_moon_condition.veins_point = moon_condition["veins_point"].cast<vector<int>>();
	return new_moon_condition;
}

PlanetCondition planet_condition_to_struct(py::dict planet_condition) {
	PlanetCondition new_planet_condition = PlanetCondition();
	new_planet_condition.satisfy_num = planet_condition["satisfy_num"].cast<uint8_t>();
	new_planet_condition.dsp_level = planet_condition["dsp_level"].cast<uint8_t>();
	new_planet_condition.type = planet_condition["type"].cast<uint8_t>();
	new_planet_condition.liquid = planet_condition["liquid"].cast<uint8_t>();
	new_planet_condition.singularity = planet_condition["singularity"].cast<uint8_t>();
	new_planet_condition.need_veins = planet_condition["need_veins"].cast<uint16_t>();
	new_planet_condition.veins_group = planet_condition["veins_group"].cast<vector<int>>();
	new_planet_condition.veins_point = planet_condition["veins_point"].cast<vector<int>>();
	auto moon_conditions = planet_condition["moons"].cast<py::list>();
	for(auto moon_condition : moon_conditions) {
		new_planet_condition.moons.push_back(moon_condition_to_struct(moon_condition.cast<py::dict>()));
	}
	return new_planet_condition;
}

StarCondition star_condition_to_struct(py::dict star_condition) {
	StarCondition new_star_condition = StarCondition();
	new_star_condition.satisfy_num = star_condition["satisfy_num"].cast<uint8_t>();
	new_star_condition.type = star_condition["type"].cast<uint8_t>();
	new_star_condition.distance = star_condition["distance"].cast<float>();
	new_star_condition.dyson_lumino = star_condition["dyson_lumino"].cast<float>();
	new_star_condition.need_veins = star_condition["need_veins"].cast<uint16_t>();
	new_star_condition.veins_group = star_condition["veins_group"].cast<vector<int>>();
	new_star_condition.veins_point = star_condition["veins_point"].cast<vector<int>>();
	auto planet_conditions = star_condition["planets"].cast<py::list>();
	for(auto planet_condition : planet_conditions) {
		new_star_condition.planets.push_back(planet_condition_to_struct(planet_condition.cast<py::dict>()));
	}
	return new_star_condition;
}

GalaxyCondition galaxy_condition_to_struct(py::dict galaxy_condition) {
	GalaxyCondition new_galaxy_condition = GalaxyCondition();
	new_galaxy_condition.need_veins = galaxy_condition["need_veins"].cast<uint16_t>();
	new_galaxy_condition.veins_group = galaxy_condition["veins_group"].cast<vector<int>>();
	new_galaxy_condition.veins_point = galaxy_condition["veins_point"].cast<vector<int>>();
	auto star_conditions = galaxy_condition["stars"].cast<py::list>();
	for(auto star_condition : star_conditions) {
		new_galaxy_condition.stars.push_back(star_condition_to_struct(star_condition.cast<py::dict>()));
	}
	auto planet_conditions = galaxy_condition["planets"].cast<py::list>();
	for(auto planet_condition : planet_conditions) {
		new_galaxy_condition.planets.push_back(planet_condition_to_struct(planet_condition.cast<py::dict>()));
	}
	return new_galaxy_condition;
}
