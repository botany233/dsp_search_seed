#pragma once
#include <string>
#include <vector>

using namespace std;

struct PlanetStruct {
	string type;
	vector<string> singularity;
	int liquid;
	bool is_gas;
	bool is_in_dsp;
	int veins[14]{0};

	//PlanetStruct():
	//	type(""),
	//	liquid(0),
	//	is_gas(false),
	//	is_in_dsp(false) {
	//	for(int i = 0; i < 14; ++i)
	//		veins[i] = 0;
	//}

	//PlanetStruct(const string& type,const vector<string>& singularity,int liquid,bool is_gas,bool is_in_dsp):
	//	type(type),
	//	singularity(singularity),
	//	liquid(liquid),
	//	is_gas(is_gas),
	//	is_in_dsp(is_in_dsp) {
	//	for(int i = 0; i < 14; ++i)
	//		veins[i] = 0;
	//}
};

struct StarStruct {
	string type;
	float distance;
	float dyson_lumino;
	vector<PlanetStruct> planets;
	int veins[14]{0};

	//StarStruct():
	//	type(""),
	//	distance(0.0f),
	//	dyson_lumino(0.0f) {
	//	for(int i = 0; i < 14; ++i)
	//		veins[i] = 0;
	//}

	//StarStruct(const string& type,float distance,float dyson_lumino):
	//	type(type),
	//	distance(distance),
	//	dyson_lumino(dyson_lumino) {
	//	for(int i = 0; i < 14; ++i)
	//		veins[i] = 0;
	//}
};

struct GalaxyStruct {
	vector<StarStruct> stars;
	int veins[14]{0};
	int planet_type_nums[23]{0};
	int star_type_nums[14]{0};

	//GalaxyStruct() {
	//	for(int i = 0; i < 14; ++i) {
	//		veins[i] = 0;
	//	}
	//}
};