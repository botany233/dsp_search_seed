#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cstdint>
#include <array>
#include <variant>

#include "data_struct.hpp"

using namespace std;
namespace py = pybind11;

const uint32_t liquid_mask[] = {
	0xFFFFFFFF,
	0x0022E281,
	0x00001000
};

static uint16_t get_need_veins(const array<uint16_t,14>& veins_point,const array<uint64_t,14>& veins_amount) {
	uint16_t result = 0;
	for(int i=0;i<14;i++) {
		result |= (uint16_t)(veins_point[i] > 0 || veins_amount[i] > 0) << i;
	}
	return result;
}

static uint16_t get_need_veins_amount(const array<uint64_t,14>& veins_amount) {
	uint16_t result = 0;
	for(int i=0;i<14;i++) {
		result |= (uint16_t)(veins_amount[i] > 0) << i;
	}
	return result;
}

static PlanetCondition planet_condition_to_struct(const py::dict& planet_condition) {
	PlanetCondition new_planet_condition = PlanetCondition();
	new_planet_condition.satisfy_num = planet_condition["satisfy_num"].cast<uint16_t>();
	new_planet_condition.dsp_level = planet_condition["dsp_level"].cast<uint8_t>();
	new_planet_condition.type = planet_condition["type"].cast<uint32_t>();
	new_planet_condition.type &= liquid_mask[planet_condition["liquid"].cast<int>()];
	new_planet_condition.singularity = planet_condition["singularity"].cast<uint8_t>();
	new_planet_condition.veins_point = planet_condition["veins_point"].cast<array<uint16_t,14>>();
	new_planet_condition.veins_amount = planet_condition["veins_amount"].cast<array<uint64_t,14>>();
	new_planet_condition.need_veins = get_need_veins(new_planet_condition.veins_point,new_planet_condition.veins_amount);
	new_planet_condition.need_veins_amount = get_need_veins_amount(new_planet_condition.veins_amount);
	auto moon_conditions = planet_condition["moons"].cast<py::list>();
	for(auto moon_condition :moon_conditions) {
		new_planet_condition.moons.push_back(planet_condition_to_struct(moon_condition.cast<py::dict>()));
	}
	return new_planet_condition;
}

static StarCondition star_condition_to_struct(const py::dict& star_condition) {
	StarCondition new_star_condition = StarCondition();
	new_star_condition.satisfy_num = star_condition["satisfy_num"].cast<uint16_t>();
	new_star_condition.type = star_condition["type"].cast<uint16_t>();
	new_star_condition.distance = star_condition["distance"].cast<float>();
	new_star_condition.dyson_lumino = star_condition["dyson_lumino"].cast<float>();
	new_star_condition.veins_point = star_condition["veins_point"].cast<array<uint16_t,14>>();
	new_star_condition.veins_amount = star_condition["veins_amount"].cast<array<uint64_t,14>>();
	new_star_condition.need_veins = get_need_veins(new_star_condition.veins_point,new_star_condition.veins_amount);
	new_star_condition.need_veins_amount = get_need_veins_amount(new_star_condition.veins_amount);
	auto planet_conditions = star_condition["planets"].cast<py::list>();
	for(auto planet_condition : planet_conditions) {
		new_star_condition.planets.push_back(planet_condition_to_struct(planet_condition.cast<py::dict>()));
	}
	return new_star_condition;
}

static BondCondition bond_condition_to_struct(const py::dict& bond_condition) {
	BondCondition new_bond_condition = BondCondition();
	new_bond_condition.distance = bond_condition["distance"].cast<double>();
	new_bond_condition.satisfy_num = bond_condition["satisfy_num"].cast<int>();
	if(bond_condition["con1_is_planet"].cast<bool>()) {
		new_bond_condition.con1 = planet_condition_to_struct(bond_condition["con1"].cast<py::dict>());
	} else {
		new_bond_condition.con1 = star_condition_to_struct(bond_condition["con1"].cast<py::dict>());
	}
	if(bond_condition["con2_is_planet"].cast<bool>()) {
		new_bond_condition.con2 = planet_condition_to_struct(bond_condition["con2"].cast<py::dict>());
	} else {
		new_bond_condition.con2 = star_condition_to_struct(bond_condition["con2"].cast<py::dict>());
	}
	return new_bond_condition;
}

GalaxyCondition galaxy_condition_to_struct(const py::dict& galaxy_condition) {
	GalaxyCondition new_galaxy_condition = GalaxyCondition();
	new_galaxy_condition.valid_state = galaxy_condition["valid_state"].cast<bool>();
	new_galaxy_condition.veins_point = galaxy_condition["veins_point"].cast<array<uint16_t,14>>();
	new_galaxy_condition.veins_amount = galaxy_condition["veins_amount"].cast<array<uint64_t,14>>();
	new_galaxy_condition.need_veins = get_need_veins(new_galaxy_condition.veins_point,new_galaxy_condition.veins_amount);
	new_galaxy_condition.need_veins_amount = get_need_veins_amount(new_galaxy_condition.veins_amount);
	auto star_conditions = galaxy_condition["stars"].cast<py::list>();
	for(auto star_condition : star_conditions) {
		new_galaxy_condition.stars.push_back(star_condition_to_struct(star_condition.cast<py::dict>()));
	}
	auto planet_conditions = galaxy_condition["planets"].cast<py::list>();
	for(auto planet_condition : planet_conditions) {
		new_galaxy_condition.planets.push_back(planet_condition_to_struct(planet_condition.cast<py::dict>()));
	}
	auto bond_conditions = galaxy_condition["bonds"].cast<py::list>();
	for(auto bond_condition : bond_conditions) {
		new_galaxy_condition.bonds.push_back(bond_condition_to_struct(bond_condition.cast<py::dict>()));
	}
	return new_galaxy_condition;
}
