#pragma once
#include <string>
#include <vector>
#include <cstdint>

using namespace std;

struct PlanetStructSimple {
	string type;
	vector<string> singularity;
	int liquid;
	bool is_gas;
	uint8_t dsp_level;
	uint16_t need_veins = 0;
	unsigned short veins[14]{0};
	unsigned short veins_point[14]{0};
};

struct StarStructSimple {
	string type;
	float distance;
	float dyson_lumino;
	vector<PlanetStructSimple> planets;
	uint16_t need_veins = 0;
	unsigned short veins[14]{0};
	unsigned short veins_point[14]{0};
};

struct GalaxyStructSimple {
	vector<StarStructSimple> stars;
	uint16_t need_veins = 0;
	unsigned short veins[14]{0};
	unsigned short veins_point[14]{0};
};

struct PlanetStruct {
	string name;
	string type;
	vector<string> singularity;
	int seed;
	float lumino;
	float wind;
	float radius;
	int liquid;
	bool is_gas;
	bool is_in_dsp;
	bool is_on_dsp;
	vector<unsigned short> veins = vector<unsigned short>(14,0);
	vector<unsigned short> veins_point = vector<unsigned short>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
};

struct StarStruct {
	string name;
	string type;
	int seed;
	float distance;
	float dyson_lumino;
	float dyson_radius;
	vector<PlanetStruct> planets;
	vector<unsigned short> veins = vector<unsigned short>(14,0);
	vector<unsigned short> veins_point = vector<unsigned short>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<unsigned short> liquid = vector<unsigned short>(3,0);
};

struct GalaxyStruct {
	int seed;
	int star_num;
	vector<StarStruct> stars;
	vector<unsigned short> veins = vector<unsigned short>(14,0);
	vector<unsigned short> veins_point = vector<unsigned short>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<unsigned short> planet_type_nums = vector<unsigned short>(23,0);
	vector<unsigned short> star_type_nums = vector<unsigned short>(14,0);
	vector<unsigned short> liquid = vector<unsigned short>(3,0);
};
