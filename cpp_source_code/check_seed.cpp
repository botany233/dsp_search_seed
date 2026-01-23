//#include <memory>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <algorithm>

#include "DSPGen.hpp"
#include "check_seed_util.hpp"
#include "data_struct.hpp"
#include "PlanetAlgorithm.hpp"
#include "PlanetRawData.hpp"

using namespace std;

uint16_t get_has_veins(const int *veins,const int *veins_point) {
	uint16_t has_veins = 0;
	for(int i=0;i<14;i++) {
		has_veins |= (veins[i] > 0 || veins_point[i] > 0) << i;
	}
	return has_veins;
}

GalaxyData get_galaxy_data(int seed,int star_num,bool quick)
{
	GalaxyData galaxy_data;
	UniverseGen g;
	g.CreateGalaxy(seed,star_num,0);
	galaxy_data.seed = seed;
	galaxy_data.star_num = star_num;
	for(StarClass star : g.stars)
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
		star_data.dyson_radius = star.dysonRadius * 2;
		star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		for(PlanetClass planet : star.planets)
		{
			PlanetData planet_data;
			planet_data.name = planet.name;
			planet_data.type = planet.display_name;
			planet_data.type_id = planet.typeId();
			planet_data.singularity = planet.GetPlanetSingularityMask();
			planet_data.singularity_str = planet.GetPlanetSingularityVector();
			planet_data.seed = planet.seed;
			planet_data.lumino = planet.luminosity;
			planet_data.wind = planet.windStrength;
			planet_data.radius = planet.orbitRadius;
			if(star.dysonRadius > planet.maxorbitRadius * 0.6770833f)
				planet_data.dsp_level = 2;
			else if((1.083333f-planet.get_ion_enhance())*planet.maxorbitRadius <= 1.6f * star.dysonRadius)
				planet_data.dsp_level = 1;
			else
				planet_data.dsp_level = 0;
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
				int veins_group[14] = {0};
				int veins_point[14] = {0};
				if(quick) {
					g.MyGenerateVeins(star,planet,veins_group,veins_point);
				}
				else {
					std::unique_ptr planet_algorithm = PlanetAlgorithmManager(planet.algoId);
					planet_algorithm->GenerateTerrain(planet,planet.mod_x,planet.mod_y);
					planet_algorithm->GenerateVeins(star,planet,g.birthPlanetId,veins_group,veins_point);
				}
				for(int i = 0; i < 14; i++)
				{
					planet_data.veins_group[i] = veins_group[i];
					planet_data.veins_point[i] = veins_point[i];
					star_data.veins_group[i] += veins_group[i];
					star_data.veins_point[i] += veins_point[i];
				}
			}
			star_data.planets.push_back(planet_data);
			//galaxy_data.planet_type_nums[planet_theme_to_type[planet.theme - 1]] += 1;
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
		//galaxy_data.star_type_nums[star.typeId()] += 1;
	}
	return galaxy_data;
}

void generate_real_veins(UniverseGen& g,StarClass& star,PlanetClass& planet,PlanetDataSimple& planet_data)
{
	memset(planet_data.veins_group,0,sizeof(planet_data.veins_group));
	memset(planet_data.veins_point,0,sizeof(planet_data.veins_point));
	std::unique_ptr planet_algorithm = PlanetAlgorithmManager(planet.algoId);
	planet_algorithm->GenerateTerrain(planet,planet.mod_x,planet.mod_y);
	planet_algorithm->GenerateVeins(star,planet,g.birthPlanetId,planet_data.veins_group,planet_data.veins_point);
	planet_data.is_real_veins = true;
}

bool check_pc_veins(const PlanetConditionSimple& planet_condition_simple,const PlanetDataSimple& planet_data)
{
	for(int i=0;i<14;i++) {
		if(planet_condition_simple.veins_point[i] > planet_data.veins_point[i] || planet_condition_simple.veins_group[i] > planet_data.veins_group[i]) {
			return false;
		}
	}
	return true;
}

bool check_sc_veins(const StarDataSimple& star_data)
{
	for(int i=0;i<14;i++) {
		if(star_data.veins_group[i] > 0 || star_data.veins_point[i] > 0) {
			return false;
		}
	}
	return true;
}

bool check_gc_veins(const GalaxyDataSimple& galaxy_data)
{
	for(int i=0;i<14;i++) {
		if(galaxy_data.veins_group[i] > 0 || galaxy_data.veins_point[i] > 0) {
			return false;
		}
	}
	return true;
}

bool check_seed_level_3(UniverseGen& g,GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition,int check_level)
{
	GalaxyConditionSimple galaxy_condition_simple = init_galaxy_condition_struct(galaxy_condition);
	get_galaxy_condition_struct(galaxy_data,galaxy_condition,galaxy_condition_simple);
	for(const PlanetConditionSimple& planet_condition_simple: galaxy_condition_simple.planets) {
		int satisfy_num = planet_condition_simple.satisfy_num;
		for(const PlanetIndexStruct& pi_struct: planet_condition_simple.planet_indexes) {
			PlanetDataSimple& planet_data = galaxy_data.stars[pi_struct.star_index].planets[pi_struct.planet_index];
			if(!planet_data.is_real_veins) {
				StarClass& star = g.stars[pi_struct.star_index];
				PlanetClass& planet = star.planets[pi_struct.planet_index];
				generate_real_veins(g,star,planet,planet_data);
			}
			if(check_pc_veins(planet_condition_simple,planet_data)) {
				satisfy_num -= 1;
				if(!satisfy_num)
					break;
			}
		}
		if(satisfy_num)
			return false;
	}
	for(const StarConditionSimple& star_condition_simple: galaxy_condition_simple.stars) {
		int star_satisfy_num = star_condition_simple.satisfy_num;
		for(const StarIndexStruct& si_struct: star_condition_simple.star_indexes) {
			int star_index = si_struct.star_index;
			StarDataSimple& star_data = galaxy_data.stars[star_index];
			if(star_condition_simple.planets.size() > 0) {
				bool satisfy_flag = false;
				for(int pc_index=0;pc_index<star_condition_simple.planets.size();pc_index++) {
					const PlanetConditionSimple& planet_condition_simple = star_condition_simple.planets[pc_index];
					int planet_satisfy_num = planet_condition_simple.satisfy_num;
					for(int planet_index: si_struct.satisfy_planets[pc_index]) {
						PlanetDataSimple& planet_data = star_data.planets[planet_index];
						if(!planet_data.is_real_veins) {
							StarClass& star = g.stars[star_index];
							PlanetClass& planet = star.planets[planet_index];
							generate_real_veins(g,star,planet,planet_data);
						}
						if(check_pc_veins(planet_condition_simple,planet_data)) {
							planet_satisfy_num -= 1;
							if(!planet_satisfy_num)
								break;
						}
					}
					if(!planet_satisfy_num) {
						satisfy_flag = true;
						break;
					}
				}
				if(!satisfy_flag)
					continue;
			}
			if(star_condition_simple.need_veins) {
				copy_n(star_condition_simple.veins_group,14,star_data.veins_group);
				copy_n(star_condition_simple.veins_point,14,star_data.veins_point);
				for(const PlanetDataSimple& planet_data: star_data.planets) {
					if(planet_data.is_real_veins && (star_condition_simple.need_veins & planet_data.has_veins)) {
						for(int i=0;i<14;i++) {
							star_data.veins_group[i] -= planet_data.veins_group[i];
							star_data.veins_point[i] -= planet_data.veins_point[i];
						}
					}
				}
				if(!check_sc_veins(star_data)) {
					bool star_satisfy_flag = false;
					for(PlanetDataSimple& planet_data: star_data.planets) {
						if(!planet_data.is_real_veins && (get_has_veins(star_data.veins_group,star_data.veins_point) & planet_data.has_veins)) {
							StarClass& star = g.stars[star_index];
							PlanetClass& planet = star.planets[planet_data.index];
							generate_real_veins(g,star,planet,planet_data);
							for(int i=0;i<14;i++) {
								star_data.veins_group[i] -= planet_data.veins_group[i];
								star_data.veins_point[i] -= planet_data.veins_point[i];
							}
							if(check_sc_veins(star_data)) {
								star_satisfy_flag = true;
								break;
							}
						}
					}
					if(star_satisfy_flag)
						star_satisfy_num -= 1;
				}
				else
					star_satisfy_num -= 1;
			}
			else
				star_satisfy_num -= 1;
			if(!star_satisfy_num)
				break;
		}
		if(star_satisfy_num)
			return false;
	}
	if(galaxy_condition_simple.need_veins) {
		copy_n(galaxy_condition_simple.veins_group,14,galaxy_data.veins_group);
		copy_n(galaxy_condition_simple.veins_point,14,galaxy_data.veins_point);
		for(const StarDataSimple& star_data: galaxy_data.stars) {
			for(const PlanetDataSimple& planet_data: star_data.planets) {
				if(planet_data.is_real_veins && (galaxy_condition_simple.need_veins & planet_data.has_veins)) {
					for(int i=0;i<14;i++) {
						galaxy_data.veins_group[i] -= planet_data.veins_group[i];
						galaxy_data.veins_point[i] -= planet_data.veins_point[i];
					}
				}
			}
		}
		if(!check_gc_veins(galaxy_data)) {
			bool galaxy_satisfy_flag = false;
			for(StarDataSimple& star_data: galaxy_data.stars) {
				for(PlanetDataSimple& planet_data: star_data.planets) {
					if(!planet_data.is_real_veins && (get_has_veins(galaxy_data.veins_group,galaxy_data.veins_point) & planet_data.has_veins)) {
						StarClass& star = g.stars[star_data.index];
						PlanetClass& planet = star.planets[planet_data.index];
						generate_real_veins(g,star,planet,planet_data);
						for(int i=0;i<14;i++) {
							galaxy_data.veins_group[i] -= planet_data.veins_group[i];
							galaxy_data.veins_point[i] -= planet_data.veins_point[i];
						}
						if(check_gc_veins(galaxy_data)) {
							galaxy_satisfy_flag = true;
							goto end_check_label;
						}
					}
				}
			}
			end_check_label:
			if(!galaxy_satisfy_flag)
				return false;
		}
	}
	return true;
}

bool check_seed_level_2(UniverseGen& g,GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition,int check_level)
{
	for(int star_index=0;star_index<g.starCount;star_index++)
	{
		StarClass& star = g.stars[star_index];
		StarDataSimple& star_data = galaxy_data.stars[star_index];
		for(int planet_index=0;planet_index<star.planetCount;planet_index++)
		{
			PlanetClass& planet = star.planets[planet_index];
			PlanetDataSimple& planet_data = star_data.planets[planet_index];
			g.MyGenerateVeins(star,planet,planet_data.veins_group,planet_data.veins_point);
			planet_data.has_veins = get_has_veins(planet_data.veins_group,planet_data.veins_point);
			for(int i=0;i<14;i++)
			{
				star_data.veins_group[i] += planet_data.veins_group[i];
				star_data.veins_point[i] += planet_data.veins_point[i];
			}
		}
		for(int i=0;i<14;i++)
		{
			galaxy_data.veins_group[i] += star_data.veins_group[i];
			galaxy_data.veins_point[i] += star_data.veins_point[i];
		}
	}
	if(!check_galaxy(galaxy_data,galaxy_condition))
		return false;
	else if(check_level>2)
		return check_seed_level_3(g,galaxy_data,galaxy_condition,check_level);
	else
		return true;
}

bool check_seed_level_1(int seed,int star_num,const GalaxyCondition& galaxy_condition,int check_level)
{
	GalaxyDataSimple galaxy_data;
	UniverseGen g;
	g.CreateGalaxy(seed,star_num,1);
	for(StarClass star : g.stars)
	{
		StarDataSimple star_data;
		star_data.type = star.typeId();
		star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data.dyson_lumino = star.dysonLumino();
		star_data.index = star.index;
		for(PlanetClass planet : star.planets)
		{
			PlanetDataSimple planet_data;
			planet_data.type = planet.typeId();
			planet_data.singularity = planet.GetPlanetSingularityMask();
			planet_data.index = planet.index;
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
