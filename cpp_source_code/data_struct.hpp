#pragma once
#include <array>
#include <string>
#include <vector>
#include <variant>
#include <cstdint>

using namespace std;

struct SeedStruct {
	int seed_id;
	uint8_t star_num;
	uint8_t resource_index;
	SeedStruct(): seed_id(0),star_num(0),resource_index(0) {}
	SeedStruct(int seed_id,uint8_t star_num,uint8_t resource_index): seed_id(seed_id),star_num(star_num),resource_index(resource_index) {}
};

struct PlanetCondition {
	uint32_t type = 0xFFFFFFFF;
	uint16_t satisfy_num = 1;
	uint8_t dsp_level = 0;
	//uint8_t type = 0;
	uint8_t liquid = 0;
	uint8_t singularity = 0;
	bool need_veins_amount = false;
	uint16_t need_veins = 0;
	array<uint16_t,14> veins_point = {};
	array<uint64_t,14> veins_amount = {};
	vector<PlanetCondition> moons = vector<PlanetCondition>();
};

struct StarCondition {
	float distance = 1000.0f;
	float dyson_lumino = 0.0f;
	uint16_t type = 0xFFFF;
	uint16_t satisfy_num = 1;
	uint16_t need_veins = 0;
	bool need_veins_amount = false;
	array<uint16_t,14> veins_point = {};
	array<uint64_t,14> veins_amount = {};
	vector<PlanetCondition> planets = vector<PlanetCondition>();
};

struct BondCondition {
	double distance = 1000.0; // 单位：LY
	int satisfy_num = 0;

	std::variant<PlanetCondition,StarCondition> con1;
	std::variant<PlanetCondition,StarCondition> con2;
};

struct GalaxyCondition {
	uint16_t need_veins = 0;
	bool need_veins_amount = false;
	array<uint16_t,14> veins_point = {};
	array<uint64_t,14> veins_amount = {};
	vector<StarCondition> stars{};
	vector<PlanetCondition> planets{};
	vector<BondCondition> bonds{};
};

struct PlanetData {
	string name;
	string type;
	int type_id;
	uint8_t singularity;
	bool is_gas;
	int seed;
	float lumino;
	float wind;
	float radius;
	int liquid;
	int dsp_level;
	array<double,3> pos_m = {};
	array<double,3> pos_ly = {};
	vector<string> singularity_str = vector<string>();
	array<uint16_t,14> veins_point = {};
	array<uint64_t,14> veins_amount = {};
	array<float,3> gas_veins = {};
	vector<PlanetData> moons = vector<PlanetData>();
};

struct StarData {
	string name;
	string type;
	int type_id;
	int seed;
	float dyson_lumino;
	float dyson_radius;
	float distance;
	array<uint16_t,14> veins_point = {};
	array<uint64_t,14> veins_amount = {};
	array<float,3> gas_veins = {};
	array<int,3> liquid = {};
	array<double,3> pos_m = {};
	array<double,3> pos_ly = {};
	vector<PlanetData> planets;
};

struct GalaxyData {
	int seed_id;
	uint8_t star_num;
	uint8_t resource_index;
	float resource_rate;
	array<uint16_t,14> veins_point = {};
	array<uint64_t,14> veins_amount = {};
	array<float,3> gas_veins = {};
	array<int,3> liquid = {};
	vector<StarData> stars;
};
