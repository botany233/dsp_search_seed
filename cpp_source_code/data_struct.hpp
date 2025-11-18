#pragma once
#include <string>
#include <vector>
#include <cstdint>

using namespace std;

struct PlanetIndexStruct {
	int star_index;
	int planet_index;
};

struct StarIndexStruct {
	int star_index;
	vector<vector<int>> satisfy_planets;
};

struct PlanetConditionSimple {
	uint16_t need_veins = 0;
	uint16_t satisfy_num;
	int veins_group[14]{0};
	int veins_point[14]{0};
	vector<PlanetIndexStruct> planet_indexes = vector<PlanetIndexStruct>();
};

struct StarConditionSimple {
	uint16_t need_veins = 0;
	uint16_t satisfy_num;
	int veins_group[14]{0};
	int veins_point[14]{0};
	vector<PlanetConditionSimple> planets = vector<PlanetConditionSimple>();
	vector<StarIndexStruct> star_indexes = vector<StarIndexStruct>();
};

struct GalaxyConditionSimple {
	uint16_t need_veins = 0;
	int veins_group[14]{0};
	int veins_point[14]{0};
	vector<StarConditionSimple> stars = vector<StarConditionSimple>();
	vector<PlanetConditionSimple> planets = vector<PlanetConditionSimple>();
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

struct PlanetStructSimple {
	uint8_t type;
	uint8_t singularity;
	uint8_t liquid;
	uint8_t dsp_level;
	bool is_real_veins = false;
	uint8_t index;
	uint16_t has_veins = 0;
	int veins_group[14]{0};
	int veins_point[14]{0};
};

struct StarStructSimple {
	uint8_t type;
	float distance;
	float dyson_lumino;
	vector<PlanetStructSimple> planets;
	uint8_t index;
	int veins_group[14]{0};
	int veins_point[14]{0};
};

struct GalaxyStructSimple {
	vector<StarStructSimple> stars;
	int veins_group[14]{0};
	int veins_point[14]{0};
};

struct PlanetStruct {
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

struct StarStruct {
	string name;
	string type;
	int type_id;
	int seed;
	float distance;
	float dyson_lumino;
	float dyson_radius;
	vector<PlanetStruct> planets;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<int> liquid = vector<int>(3,0);
};

struct GalaxyStruct {
	int seed;
	int star_num;
	vector<StarStruct> stars;
	vector<int> veins_group = vector<int>(14,0);
	vector<int> veins_point = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	//vector<int> planet_type_nums = vector<int>(23,0);
	//vector<int> star_type_nums = vector<int>(14,0);
	vector<int> liquid = vector<int>(3,0);
};
