#pragma once
#include <string>
#include <vector>
#include <cstdint>

using namespace std;

struct SeedStruct {
	int seed_id;
	int star_num;
	SeedStruct(): seed_id(0),star_num(0) {}
	SeedStruct(int seed_id,int star_num): seed_id(seed_id),star_num(star_num) {}
};

struct PlanetCondition {
	uint8_t satisfy_num = 1;
	uint8_t dsp_level = 0;
	uint8_t type = 0;
	uint8_t liquid = 0;
	uint8_t singularity = 0;
	uint16_t need_veins = 0;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<PlanetCondition> moons = vector<PlanetCondition>();
};

struct StarCondition {
	uint8_t satisfy_num = 1;
	uint8_t type = 0;
	float distance = 1000.0f;
	float dyson_lumino = 0.0f;
	uint16_t need_veins = 0;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<PlanetCondition> planets = vector<PlanetCondition>();
};

struct GalaxyCondition {
	uint16_t need_veins = 0;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<StarCondition> stars = vector<StarCondition>();
	vector<PlanetCondition> planets = vector<PlanetCondition>();
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
	vector<string> singularity_str = vector<string>();
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
};

struct StarData {
	string name;
	string type;
	int type_id;
	int seed;
	float dyson_lumino;
	float dyson_radius;
	float distance;
	vector<double> pos = vector<double>(3,0);
	vector<PlanetData> planets;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<int> liquid = vector<int>(3,0);
};

struct GalaxyData {
	int seed;
	int star_num;
	vector<StarData> stars;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<int> liquid = vector<int>(3,0);
};
