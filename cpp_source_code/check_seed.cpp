//#include <memory>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <memory>

#include "check_seed_util.hpp"
#include "data_struct.hpp"
#include "PlanetAlgorithm.hpp"
#include "PlanetRawData.hpp"
#include "astro_class.hpp"
#include "DSPGen.hpp"
#include "defines.hpp"

//extern std::unique_ptr<PlanetAlgorithm> PlanetAlgorithmManager(int algoId);

using namespace std;

static uint16_t get_has_veins(const uint16_t *veins_group,const uint16_t *veins_point) {
	uint16_t has_veins = 0;
	for(int i=0;i<14;i++) {
		has_veins |= (veins_group[i] > 0 || veins_point[i] > 0) << i;
	}
	return has_veins;
}

GalaxyData get_galaxy_data(int seed,int star_num,bool quick)
{
	GalaxyData galaxy_data;
	GalaxyClass g;
	g.CreateGalaxy(seed,star_num,0);
	galaxy_data.seed = seed;
	galaxy_data.star_num = star_num;
	for(StarClass& star : g.stars)
	{
		StarData star_data;
		star_data.name = star.name;
		star_data.type = star.typeString();
		star_data.type_id = star.typeId();
		star_data.seed = star.seed;
		star_data.pos[0] = star.uPosition.x;
		star_data.pos[1] = star.uPosition.y;
		star_data.pos[2] = star.uPosition.z;
		star_data.dyson_lumino = star.dysonLumino();
		star_data.dyson_radius = round(star.dysonRadius * 800) * 100;
		star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		for(PlanetClass& planet : star.planets)
		{
			PlanetData planet_data;
			planet_data.name = planet.name;
			planet_data.type = planet.display_name;
			planet_data.type_id = planet.typeId();
			planet_data.singularity = planet.singularity;
			planet_data.singularity_str = planet.GetPlanetSingularityVector();
			planet_data.seed = planet.seed;
			planet_data.lumino = planet.luminosity;
			planet_data.wind = planet.windStrength;
			planet_data.radius = planet.orbitRadius;
			if(star_data.dyson_radius > planet.maxorbitRadius * 52083.333f)
				planet_data.dsp_level = 2;
			else if((1.041667f-planet.get_ion_enhance())*planet.maxorbitRadius <= 0.00002f * star_data.dyson_radius)
				planet_data.dsp_level = 1;
			else
				planet_data.dsp_level = 0;
			planet_data.liquid = planet.waterItemId;
			star_data.liquid[planet.waterItemId] += 1;
			if(planet.gasItems.size()) {
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
			} else {
				planet_data.is_gas = false;
				int veins_group[14] = {0};
				int veins_point[14] = {0};
				if(quick) {
					g.MyGenerateVeins(star,planet,veins_group,veins_point);
				}
				else {
					std::unique_ptr planet_algorithm = PlanetAlgorithmManager(planet.algoId);
					planet_algorithm->get_veins(star,planet,g.birthPlanetId,veins_group,veins_point);
				}
				for(int i = 0; i < 14; i++) {
					planet_data.veins_group[i] = veins_group[i];
					planet_data.veins_point[i] = veins_point[i];
					star_data.veins_group[i] += veins_group[i];
					star_data.veins_point[i] += veins_point[i];
				}
			}
			star_data.planets.push_back(planet_data);
		}
		for(int i = 0; i < 14; i++)
		{
			galaxy_data.veins_group[i] += star_data.veins_group[i];
			galaxy_data.veins_point[i] += star_data.veins_point[i];
		}
		for(int i = 0; i < 3; i++)
		{
			galaxy_data.gas_veins[i] += star_data.gas_veins[i];
			galaxy_data.liquid[i] += star_data.liquid[i];
		}
		galaxy_data.stars.push_back(star_data);
	}
	return galaxy_data;
}

bool check_seed_level_4(GalaxyClassSimple& galaxy,const GalaxyCondition& galaxy_condition,int check_level)
{
	//cout << galaxy.seed << " " << galaxy.starCount << " level4 check start" << endl;
	memset(galaxy.veins_group,0,sizeof(galaxy.veins_group));
	memset(galaxy.veins_point,0,sizeof(galaxy.veins_point));
	return check_galaxy_level_4(galaxy,galaxy_condition);
}

bool check_seed_level_3(GalaxyClassSimple& galaxy,const GalaxyCondition& galaxy_condition,int check_level)
{
	//cout << galaxy.seed << " " << galaxy.starCount << " level3 check start" << endl;
	for(StarClassSimple& star : galaxy.stars)
	{
		for(PlanetClassSimple& planet : star.planets)
		{
			if(planet.type == EPlanetType::Gas)
				planet.is_real_veins = true;
			else
				planet.MyGenerateVeins();
		}
		star.has_veins = get_has_veins(star.upper_veins_group,star.upper_veins_point);
	}
	if(!check_galaxy_level_3(galaxy,galaxy_condition))
		return false;
	else if(check_level>3)
		return check_seed_level_4(galaxy,galaxy_condition,check_level);
	else
		return true;
}

bool check_seed_level_2(GalaxyClassSimple& galaxy,const GalaxyCondition& galaxy_condition,int check_level)
{
	//cout << galaxy.seed << " " << galaxy.starCount << " level2 check start" << endl;
	galaxy.CreatePlanets();
	if(!check_galaxy_level_2(galaxy,galaxy_condition))
		return false;
	else if(check_level>2)
		return check_seed_level_3(galaxy,galaxy_condition,check_level);
	else
		return true;
}

bool check_seed_level_1(int seed,int star_num,const GalaxyCondition& galaxy_condition,int check_level)
{
	GalaxyClassSimple galaxy;
	galaxy.CreateStars(seed,star_num);
	if(!check_galaxy_level_1(galaxy,galaxy_condition))
		return false;
	else if(check_level>1)
		return check_seed_level_2(galaxy,galaxy_condition,check_level);
	else
		return true;
}
