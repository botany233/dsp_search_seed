//#include <memory>
#include <cstring>
#include <cstdint>
#include <iostream>
#include "json.hpp"
#include <bitset>

#include "DSPGen.hpp"
#include "check_seed_util.hpp"
#include "data_struct.hpp"
#include "PlanetAlgorithm.hpp"
#include "PlanetRawData.hpp"

using namespace nlohmann;
using namespace std;

const int planet_theme_to_type[25] = {
	0, 22, 22, 1, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	21, 17, 18, 19, 20
};

uint16_t get_has_veins(const uint16_t *veins,const uint16_t *veins_point) {
	uint16_t has_veins = 0;
	for(int i=0;i<14;i++) {
		has_veins |= (veins[i] > 0 || veins_point[i] > 0) << i;
	}
	return has_veins;
}

GalaxyStruct get_galaxy_data(int seed,int star_num)
{
	GalaxyStruct galaxy_data;
	UniverseGen g;
	g.CreateGalaxy(seed,star_num,0);
	galaxy_data.seed = seed;
	galaxy_data.star_num = star_num;
	for(StarData star : g.stars)
	{
		StarStruct star_data;
		star_data.name = star.name;
		star_data.type = star.typeString();
		star_data.seed = star.seed;
		star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data.dyson_lumino = star.dysonLumino();
		star_data.dyson_radius = star.dysonRadius * 2;
		for(PlanetData planet : star.planets)
		{
			PlanetStruct planet_data;
			planet_data.name = planet.name;
			planet_data.type = planet.display_name;
			planet_data.singularity = planet.GetPlanetSingularityVector();
			planet_data.seed = planet.seed;
			planet_data.lumino = planet.luminosity;
			planet_data.wind = planet.windStrength;
			planet_data.radius = planet.orbitRadius;
			planet_data.is_in_dsp = star.dysonRadius > planet.maxorbitRadius * 0.6770833f;
			planet_data.is_on_dsp = (1.083333f-planet.get_ion_enhance())*planet.maxorbitRadius <= 1.6f * star.dysonRadius;
			if(planet.waterItemId == 1000)
			{
				planet_data.liquid = 1;
				star_data.liquid[1] += 1;
			}
			else if(planet.waterItemId == 1116)
			{
				planet_data.liquid = 2;
				star_data.liquid[2] += 1;
			}
			else
			{
				planet_data.liquid = 0;
				star_data.liquid[0] += 1;
			}
			if(planet.gasItems.size())
			{
				planet_data.is_gas = true;
				for(int i = 0; i < 2; i++)
				{
					if(planet.gasItems[i] == 1120)
						planet_data.gas_veins[0] = planet.gasSpeeds[i];
					else if(planet.gasItems[i] == 1121)
						planet_data.gas_veins[1] = planet.gasSpeeds[i];
					else if(planet.gasItems[i] == 1011)
						planet_data.gas_veins[2] = planet.gasSpeeds[i];
				}
				for(int i = 0; i < 3; i++)
					star_data.gas_veins[i] += planet_data.gas_veins[i];
			}
			else
			{
				planet_data.is_gas = false;
				unsigned short veins_group[14] = {0};
				unsigned short veins_point[14] = {0};
				std::unique_ptr planet_algorithm = PlanetAlgorithmManager(planet.algoId);
				planet_algorithm->GenerateTerrain(planet,planet.mod_x,planet.mod_y);
				planet_algorithm->GenerateVeins(star,planet,g.birthPlanetId,veins_group,veins_point);
				for(int i = 0; i < 14; i++)
				{
					planet_data.veins[i] = veins_group[i];
					planet_data.veins_point[i] = veins_point[i];
					star_data.veins[i] += veins_group[i];
					star_data.veins_point[i] += veins_point[i];
				}
			}
			star_data.planets.push_back(planet_data);
			galaxy_data.planet_type_nums[planet_theme_to_type[planet.theme - 1]] += 1;
		}
		for(int i = 0; i < 14; i++)
		{
			galaxy_data.veins[i] += star_data.veins[i];
			galaxy_data.veins_point[i] += star_data.veins_point[i];
		}
		for(int i = 0; i < 3; i++)
		{
			galaxy_data.gas_veins[i] += star_data.gas_veins[i];
			galaxy_data.liquid[i] += star_data.liquid[i];
		}
		galaxy_data.stars.push_back(star_data);
		galaxy_data.star_type_nums[star.typeId()] += 1;
	}
	return galaxy_data;
}

bool check_seed_level_3(UniverseGen& g,GalaxyStructSimple& galaxy_data,const json& galaxy_condition,int check_level)
{
	uint16_t galaxy_need_veins = galaxy_data.need_veins;
	memset(galaxy_data.veins,0,sizeof(galaxy_data.veins));
	memset(galaxy_data.veins_point,0,sizeof(galaxy_data.veins_point));
	for(int star_index=0;star_index<g.starCount;star_index++)
	{
		StarData& star = g.stars[star_index];
		StarStructSimple& star_data = galaxy_data.stars[star_index];
		uint16_t star_need_veins = star_data.need_veins | galaxy_need_veins;
		memset(star_data.veins,0,sizeof(star_data.veins));
		memset(star_data.veins_point,0,sizeof(star_data.veins_point));
		for(int planet_index=0;planet_index<star.planetCount;planet_index++)
		{
			PlanetData& planet = star.planets[planet_index];
			PlanetStructSimple& planet_data = star_data.planets[planet_index];
			uint16_t planet_need_veins = planet_data.need_veins | star_need_veins;
			uint16_t planet_has_veins = get_has_veins(planet_data.veins,planet_data.veins_point);
			memset(planet_data.veins,0,sizeof(planet_data.veins));
			memset(planet_data.veins_point,0,sizeof(planet_data.veins_point));
			if(!(planet_need_veins & planet_has_veins))
				continue;
			std::unique_ptr planet_algorithm = PlanetAlgorithmManager(planet.algoId);
			planet_algorithm->GenerateTerrain(planet,planet.mod_x,planet.mod_y);
			planet_algorithm->GenerateVeins(star,planet,g.birthPlanetId,planet_data.veins,planet_data.veins_point);
			for(int i=0;i<14;i++)
			{
				star_data.veins[i] += planet_data.veins[i];
				star_data.veins_point[i] += planet_data.veins_point[i];
			}
		}
		for(int i=0;i<14;i++)
		{
			galaxy_data.veins[i] += star_data.veins[i];
			galaxy_data.veins_point[i] += star_data.veins_point[i];
		}
	}
	return check_galaxy(galaxy_data,galaxy_condition,false);
}

bool check_seed_level_2(UniverseGen& g,GalaxyStructSimple& galaxy_data,const json& galaxy_condition,int check_level)
{
	for(int star_index=0;star_index<g.starCount;star_index++)
	{
		StarData& star = g.stars[star_index];
		StarStructSimple& star_data = galaxy_data.stars[star_index];
		for(int planet_index=0;planet_index<star.planetCount;planet_index++)
		{
			PlanetData& planet = star.planets[planet_index];
			PlanetStructSimple& planet_data = star_data.planets[planet_index];
			g.MyGenerateVeins(star,planet,planet_data.veins,planet_data.veins_point);
			for(int i=0;i<14;i++)
			{
				star_data.veins[i] += planet_data.veins[i];
				star_data.veins_point[i] += planet_data.veins_point[i];
			}
		}
		for(int i=0;i<14;i++)
		{
			galaxy_data.veins[i] += star_data.veins[i];
			galaxy_data.veins_point[i] += star_data.veins_point[i];
		}
	}
	if(!check_galaxy(galaxy_data,galaxy_condition,check_level>2))
		return false;
	else if(check_level>2)
		return check_seed_level_3(g,galaxy_data,galaxy_condition,check_level);
	else
		return true;
}

bool check_seed_level_1(int seed,int star_num,const json& galaxy_condition,int check_level)
{
	GalaxyStructSimple galaxy_data;
	UniverseGen g;
	g.CreateGalaxy(seed,star_num,1);
	for(StarData star : g.stars)
	{
		StarStructSimple star_data;
		star_data.type = star.typeString();
		star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data.dyson_lumino = star.dysonLumino();
		for(PlanetData planet : star.planets)
		{
			PlanetStructSimple planet_data;
			planet_data.type = planet.display_name;
			planet_data.singularity = planet.GetPlanetSingularityVector();
			if(star.dysonRadius > planet.maxorbitRadius * 0.6770833f)
				planet_data.dsp_level = 2;
			else if((1.083333f-planet.get_ion_enhance())*planet.maxorbitRadius <= 1.6f * star.dysonRadius)
				planet_data.dsp_level = 1;
			else
				planet_data.dsp_level = 0;
			if(planet.waterItemId == 1000)
				planet_data.liquid = 1;//水
			else if(planet.waterItemId == 1116)
				planet_data.liquid = 2;//硫酸
			else
				planet_data.liquid = 0;//无
			planet_data.is_gas = planet.gasItems.size();
			star_data.planets.push_back(planet_data);
		}
		galaxy_data.stars.push_back(star_data);
	}
	if(!check_galaxy_quick(galaxy_data,galaxy_condition))
		return false;
	else if(check_level>1)
		return check_seed_level_2(g,galaxy_data,galaxy_condition,check_level);
	else
		return true;
}
