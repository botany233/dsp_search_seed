#pragma once
#include <string>
#include <vector>
//#include <array>

using namespace std;

struct PlanetStructSimple {
	string type;
	vector<string> singularity;
	int liquid;
	bool is_gas;
	bool is_in_dsp;
	bool is_on_dsp;
	int veins[14]{0};
};

struct StarStructSimple {
	string type;
	float distance;
	float dyson_lumino;
	vector<PlanetStructSimple> planets;
	int veins[14]{0};
};

struct GalaxyStructSimple {
	vector<StarStructSimple> stars;
	int veins[14]{0};
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
	vector<int> veins = vector<int>(14,0);
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
	vector<int> veins = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<int> liquid = vector<int>(3,0);
};

struct GalaxyStruct {
	int seed;
	int star_num;
	vector<StarStruct> stars;
	vector<int> veins = vector<int>(14,0);
	vector<float> gas_veins = vector<float>(3,0);
	vector<int> planet_type_nums = vector<int>(23,0);
	vector<int> star_type_nums = vector<int>(14,0);
	vector<int> liquid = vector<int>(3,0);
};
