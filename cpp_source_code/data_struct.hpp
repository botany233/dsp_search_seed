#pragma once
#include <array>
#include <string>
#include <vector>
#include <variant>
#include <cstdint>

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
	uint8_t singularity = 0;
	bool need_veins_amount = false;
	uint16_t need_veins = 0;
	std::array<uint16_t,14> veins_point = {};
	std::array<uint64_t,14> veins_amount = {};
	std::vector<PlanetCondition> moons = std::vector<PlanetCondition>();
};

struct StarCondition {
	float distance = 1000.0f;
	float dyson_lumino = 0.0f;
	uint16_t type = 0xFFFF;
	uint16_t satisfy_num = 1;
	uint16_t need_veins = 0;
	bool need_veins_amount = false;
	std::array<uint16_t,14> veins_point = {};
	std::array<uint64_t,14> veins_amount = {};
	std::vector<PlanetCondition> planets = std::vector<PlanetCondition>();
};

struct BondCondition {
	double distance = 1000.0; // 单位：LY
	int satisfy_num = 0;

	std::variant<PlanetCondition,StarCondition> con1;
	std::variant<PlanetCondition,StarCondition> con2;
};

struct GalaxyCondition {
	uint16_t need_veins = 0;
	bool valid_state = true;
	bool need_gen_position = true;
	bool need_veins_amount = false;
	std::array<uint16_t,14> veins_point = {};
	std::array<uint64_t,14> veins_amount = {};
	std::vector<StarCondition> stars{};
	std::vector<PlanetCondition> planets{};
	std::vector<BondCondition> bonds{};
};

struct PlanetData {
	int star_index;
	int planet_index;
	std::string name_enus;
	std::string name_zhcn;
	std::string type;
	int type_id;
	uint8_t singularity;
	bool is_gas;
	int seed;
	float lumino;
	float wind;
	float radius;
	int liquid;
	int dsp_level;
	float raw_dsp_degree;
	float enhance_dsp_degree;
	float obliquity = 0.0f;
	float land_percent = 0.0f;
	std::array<double,3> pos_m = {};
	std::array<double,3> pos_ly = {};
	std::vector<std::string> singularity_str = std::vector<std::string>();
	std::array<uint16_t,14> veins_point = {};
	std::array<uint64_t,14> veins_amount = {};
	std::array<float,3> gas_veins = {};
	std::vector<PlanetData> moons = std::vector<PlanetData>();
};

struct StarData {
	int star_index;
	std::string name_enus;
	std::string name_zhcn;
	std::string type;
	int type_id;
	int seed;
	float dyson_lumino;
	float dyson_radius;
	float distance;
	std::array<uint16_t,14> veins_point = {};
	std::array<uint64_t,14> veins_amount = {};
	std::array<float,3> gas_veins = {};
	std::array<int,3> liquid = {};
	std::array<double,3> pos_m = {};
	std::array<double,3> pos_ly = {};
	std::vector<PlanetData> planets;
};

struct GalaxyData {
	int seed_id;
	uint8_t star_num;
	uint8_t resource_index;
	float resource_rate;
	std::array<uint16_t,14> veins_point = {};
	std::array<uint64_t,14> veins_amount = {};
	std::array<float,3> gas_veins = {};
	std::array<int,3> liquid = {};
	std::vector<StarData> stars;
};
