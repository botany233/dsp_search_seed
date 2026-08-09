#include <cstring>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <atomic>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "check_seed_util.hpp"
#include "data_struct.hpp"
#include "PlanetAlgorithm.hpp"
#include "astro_class.hpp"
#include "DSPGen.hpp"
#include "defines.hpp"
#include "LDB.hpp"

using namespace std;

GalaxyData get_galaxy_data_para(const SeedStruct& seed,int thread_num) {
	GalaxyClass g;
	float resource_rate = resource_rates[seed.resource_index];
	g.CreateStars(seed.seed_id,seed.star_num,resource_rate);
	g.CreatePlanets();
	thread_num = clamp(thread_num,1,8);

	vector<pair<StarClass*,PlanetClass*>> planet_tasks;
	for(StarClass& star : g.stars) {
		for(PlanetClass& planet : star.planets) {
			if(planet.gasItems.empty())
				planet_tasks.emplace_back(&star,&planet);
		}
	}

	if(!planet_tasks.empty()) {
		atomic<size_t> next_task = 0;
		auto worker = [&]() {
			while(true) {
				size_t task_index = next_task.fetch_add(1);
				if(task_index >= planet_tasks.size())
					break;

				auto [star,planet] = planet_tasks[task_index];
				unique_ptr<PlanetAlgorithm> planet_algorithm = GetPlanetAlgorithm(planet->algoId);
				planet_algorithm->get_veins(g,*star,*planet);
			}
		};

		size_t worker_num = min<size_t>(thread_num,planet_tasks.size());
		if(worker_num == 1) {
			worker();
		} else {
			vector<jthread> workers;
			workers.reserve(worker_num);
			for(size_t i=0;i<worker_num;i++)
				workers.emplace_back(worker);
		}
	}

	GalaxyData galaxy_data;
	galaxy_data.seed_id = seed.seed_id;
	galaxy_data.star_num = seed.star_num;
	galaxy_data.resource_rate = resource_rate;
	galaxy_data.resource_index = seed.resource_index;
	galaxy_data.stars.reserve(seed.star_num);
	for(const StarClass& star : g.stars)
	{
		StarData star_data;
		star_data.star_index = star.index;
		star_data.name = star.name;
		star_data.type = star.typeString();
		star_data.type_id = star.typeId();
		star_data.seed = star.seed;
		star_data.pos_m[0] = star.uPosition.x;
		star_data.pos_m[1] = star.uPosition.y;
		star_data.pos_m[2] = star.uPosition.z;
		star_data.pos_ly[0] = star.position.x;
		star_data.pos_ly[1] = star.position.y;
		star_data.pos_ly[2] = star.position.z;
		star_data.dyson_lumino = star.dysonLumino();
		star_data.dyson_radius = round(star.dysonRadius * 800) * 100;
		//star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data.distance = (star.position - g.stars[0].position).magnitude();
		star_data.planets.reserve(star.planets.size());
		for(const PlanetClass& planet : star.planets)
		{
			PlanetData planet_data;
			planet_data.star_index = star.index;
			planet_data.planet_index = planet.index;
			planet_data.name = planet.name;
			planet_data.type = planet.display_name;
			planet_data.type_id = planet.type_id;
			planet_data.singularity = planet.singularity;
			planet_data.singularity_str = planet.GetPlanetSingularityVector();
			planet_data.seed = planet.seed;
			planet_data.pos_m[0] = star.uPosition.x;
			planet_data.pos_m[1] = star.uPosition.y;
			planet_data.pos_m[2] = star.uPosition.z;
			planet_data.pos_ly[0] = star.position.x;
			planet_data.pos_ly[1] = star.position.y;
			planet_data.pos_ly[2] = star.position.z;
			planet_data.lumino = planet.luminosity;
			planet_data.wind = planet.windStrength;
			planet_data.radius = planet.orbitRadius;
			planet_data.obliquity = planet.obliquity;
			float need_dot = 1.0f / 24.0f - 0.00002f * star_data.dyson_radius / planet.maxorbitRadius;
			if(need_dot <= -1.0f)
				planet_data.raw_dsp_degree = 0.0f;
			else if(need_dot > 0.0f)
				planet_data.raw_dsp_degree = 90.0f;
			else
				planet_data.raw_dsp_degree = min(acos(-need_dot) * 57.2957795f + abs(planet.obliquity),90.0f);
			need_dot -= planet.get_ion_enhance();
			if(need_dot <= -1.0f)
				planet_data.enhance_dsp_degree = 0.0f;
			else if(need_dot > 0.0f)
				planet_data.enhance_dsp_degree = 90.0f;
			else
				planet_data.enhance_dsp_degree = min(acos(-need_dot) * 57.2957795f + abs(planet.obliquity),90.0f);
			if(planet_data.raw_dsp_degree <= 0.0f)
				planet_data.dsp_level = 2;
			else if(planet_data.enhance_dsp_degree <= 0.0f)
				planet_data.dsp_level = 1;
			else
				planet_data.dsp_level = 0;
			int real_waterItemId = planet.waterItemId & 3;
			planet_data.liquid = real_waterItemId;
			star_data.liquid[real_waterItemId] += 1;
			if(planet.gasItems.size()) {
				planet_data.is_gas = true;
				for(int i = 0; i < 2; i++) {
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
				planet_data.land_percent = planet.landPercent;
				for(int i = 0; i < 14; i++) {
					planet_data.veins_point[i] = planet.veins_point[i];
					planet_data.veins_amount[i] = planet.veins_amount[i];
					star_data.veins_point[i] += planet.veins_point[i];
					star_data.veins_amount[i] += planet.veins_amount[i];
				}
			}
			star_data.planets.push_back(planet_data);
		}
		for(int i = 0; i < 14; i++) {
			galaxy_data.veins_point[i] += star_data.veins_point[i];
			galaxy_data.veins_amount[i] += star_data.veins_amount[i];
		}
		for(int i = 0; i < 3; i++) {
			galaxy_data.gas_veins[i] += star_data.gas_veins[i];
			galaxy_data.liquid[i] += star_data.liquid[i];
		}
		galaxy_data.stars.push_back(star_data);
	}
	for(StarData& star_data: galaxy_data.stars) {
		PlanetData* last_gas=nullptr;
		for(PlanetData& planet_data: star_data.planets) {
			if(planet_data.is_gas) {
				last_gas = &planet_data;
			} else if(planet_data.singularity & EPlanetSingularity::Satellite) {
				last_gas->moons.push_back(planet_data);
			}
		}
	}
	return galaxy_data;
}

GalaxyData get_galaxy_data(const SeedStruct& seed,bool quick)
{
	GalaxyData galaxy_data;
	GalaxyClass g;
	float resource_rate = resource_rates[seed.resource_index];
	g.CreateStars(seed.seed_id,seed.star_num,resource_rate);
	g.CreatePlanets();
	galaxy_data.seed_id = seed.seed_id;
	galaxy_data.star_num = seed.star_num;
	galaxy_data.resource_rate = resource_rate;
	galaxy_data.resource_index = seed.resource_index;
	galaxy_data.stars.reserve(seed.star_num);
	for(StarClass& star : g.stars)
	{
		StarData star_data;
		star_data.star_index = star.index;
		star_data.name = star.name;
		star_data.type = star.typeString();
		star_data.type_id = star.typeId();
		star_data.seed = star.seed;
		star_data.pos_m[0] = star.uPosition.x;
		star_data.pos_m[1] = star.uPosition.y;
		star_data.pos_m[2] = star.uPosition.z;
		star_data.pos_ly[0] = star.position.x;
		star_data.pos_ly[1] = star.position.y;
		star_data.pos_ly[2] = star.position.z;
		star_data.dyson_lumino = star.dysonLumino();
		star_data.dyson_radius = round(star.dysonRadius * 800) * 100;
		//star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data.distance = (star.position - g.stars[0].position).magnitude();
		star_data.planets.reserve(star.planets.size());
		for(PlanetClass& planet : star.planets)
		{
			PlanetData planet_data;
			planet_data.star_index = star.index;
			planet_data.planet_index = planet.index;
			planet_data.name = planet.name;
			planet_data.type = planet.display_name;
			planet_data.type_id = planet.type_id;
			planet_data.singularity = planet.singularity;
			planet_data.singularity_str = planet.GetPlanetSingularityVector();
			planet_data.seed = planet.seed;
			planet_data.pos_m[0] = star.uPosition.x;
			planet_data.pos_m[1] = star.uPosition.y;
			planet_data.pos_m[2] = star.uPosition.z;
			planet_data.pos_ly[0] = star.position.x;
			planet_data.pos_ly[1] = star.position.y;
			planet_data.pos_ly[2] = star.position.z;
			planet_data.lumino = planet.luminosity;
			planet_data.wind = planet.windStrength;
			planet_data.radius = planet.orbitRadius;
			planet_data.obliquity = planet.obliquity;
			float need_dot = 1.0f / 24.0f - 0.00002f * star_data.dyson_radius / planet.maxorbitRadius;
			if(need_dot <= -1.0f)
				planet_data.raw_dsp_degree = 0.0f;
			else if(need_dot > 0.0f)
				planet_data.raw_dsp_degree = 90.0f;
			else
				planet_data.raw_dsp_degree = min(acos(-need_dot) * 57.2957795f + abs(planet.obliquity),90.0f);
			need_dot -= planet.get_ion_enhance();
			if(need_dot <= -1.0f)
				planet_data.enhance_dsp_degree = 0.0f;
			else if(need_dot > 0.0f)
				planet_data.enhance_dsp_degree = 90.0f;
			else
				planet_data.enhance_dsp_degree = min(acos(-need_dot) * 57.2957795f + abs(planet.obliquity),90.0f);
			if(planet_data.raw_dsp_degree <= 0.0f)
				planet_data.dsp_level = 2;
			else if(planet_data.enhance_dsp_degree <= 0.0f)
				planet_data.dsp_level = 1;
			else
				planet_data.dsp_level = 0;
			int real_waterItemId = planet.waterItemId & 3;
			planet_data.liquid = real_waterItemId;
			star_data.liquid[real_waterItemId] += 1;
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
				if(quick) {
					g.MyGenerateVeins(star,planet);
					planet_data.land_percent = 0.0f;
				} else {
					unique_ptr<PlanetAlgorithm> planet_algorithm = GetPlanetAlgorithm(planet.algoId);
					planet_algorithm->get_veins(g,star,planet);
					planet_data.land_percent = planet.landPercent;
				}
				for(int i = 0; i < 14; i++) {
					planet_data.veins_point[i] = planet.veins_point[i];
					planet_data.veins_amount[i] = planet.veins_amount[i];
					star_data.veins_point[i] += planet.veins_point[i];
					star_data.veins_amount[i] += planet.veins_amount[i];
				}
			}
			star_data.planets.push_back(planet_data);
		}
		for(int i = 0; i < 14; i++) {
			galaxy_data.veins_point[i] += star_data.veins_point[i];
			galaxy_data.veins_amount[i] += star_data.veins_amount[i];
		}
		for(int i = 0; i < 3; i++) {
			galaxy_data.gas_veins[i] += star_data.gas_veins[i];
			galaxy_data.liquid[i] += star_data.liquid[i];
		}
		galaxy_data.stars.push_back(star_data);
	}
	for(StarData& star_data: galaxy_data.stars) {
		PlanetData* last_gas=nullptr;
		for(PlanetData& planet_data: star_data.planets) {
			if(planet_data.is_gas) {
				last_gas = &planet_data;
			} else if(planet_data.singularity & EPlanetSingularity::Satellite) {
				last_gas->moons.push_back(planet_data);
			}
		}
	}
	return galaxy_data;
}

GalaxyData get_galaxy_data_fast(const SeedStruct& seed,bool quick,int gen_planet_num=-1,unsigned short vein_mask=0x3FFF)
{
	GalaxyClassSimple galaxy;
	galaxy.CreateStars(seed.seed_id,seed.star_num,resource_rates[seed.resource_index]);
	galaxy.CreatePlanets((gen_planet_num<0)?seed.star_num:gen_planet_num);
	if(quick) {
		for(PlanetClassSimple& planet: galaxy.planets) {
			if(planet.has_veins & vein_mask) {
				planet.need_generate_veins = true;
				planet.need_generate_veins_amount = true;
			}
		}
		galaxy.GenerateUpperVeins();
		for(StarClassSimple& star: galaxy.stars) {
			memcpy(star.real_veins_point,star.upper_veins_point,sizeof(star.upper_veins_point));
			memcpy(star.real_veins_amount,star.upper_veins_amount,sizeof(star.upper_veins_amount));
		}
	} else {
		for(PlanetClassSimple& planet: galaxy.planets)
			if(planet.has_veins & vein_mask)
				planet.generate_real_veins();
		for(StarClassSimple& star: galaxy.stars) {
			memcpy(star.upper_veins_point,star.real_veins_point,sizeof(star.real_veins_point));
			memcpy(star.upper_veins_amount,star.real_veins_amount,sizeof(star.real_veins_amount));
		}
	}

	GalaxyData galaxy_data;
	galaxy_data.seed_id = seed.seed_id;
	galaxy_data.star_num = seed.star_num;
	galaxy_data.resource_index = seed.resource_index;
	galaxy_data.resource_rate = galaxy.resource_multiplier;
	for(int i=0;i<14;i++) {
		galaxy_data.veins_point[i] = galaxy.veins_point[i];
		galaxy_data.veins_amount[i] = galaxy.veins_amount[i];
	}
	galaxy_data.stars.reserve(seed.star_num);
	for(const StarClassSimple& star: galaxy.stars) {
		StarData star_data;
		star_data.type_id = star.typeId();
		star_data.type = StarClass::type_names[star_data.type_id];
		star_data.seed = star.seed;
		star_data.pos_m[0] = star.uPosition.x;
		star_data.pos_m[1] = star.uPosition.y;
		star_data.pos_m[2] = star.uPosition.z;
		star_data.pos_ly[0] = star.position.x;
		star_data.pos_ly[1] = star.position.y;
		star_data.pos_ly[2] = star.position.z;
		star_data.dyson_radius = star.dysonRadius;
		star_data.dyson_lumino = star.luminosity;
		star_data.distance = (star.position - galaxy.stars[0].position).magnitude();
		for(int i=0;i<14;i++) {
			star_data.veins_point[i] = star.real_veins_point[i];
			star_data.veins_amount[i] = star.real_veins_amount[i];
		}
		star_data.planets.reserve(star.planets.size());
		for(const PlanetClassSimple& planet: star.planets) {
			PlanetData planet_data;
			const ThemeProto& theme = LDB.Select(planet.theme);
			planet_data.type = theme.DisplayName;
			planet_data.type_id = theme.TypeId;
			planet_data.singularity = planet.singularity;
			planet_data.is_gas = planet.type == EPlanetType::Gas;
			planet_data.seed = planet.seed;
			planet_data.liquid = planet.waterItemId & 3;
			planet_data.dsp_level = planet.dsp_level;
			planet_data.radius = planet.orbitRadius;
			planet_data.pos_m[0] = planet.star->uPosition.x;
			planet_data.pos_m[1] = planet.star->uPosition.y;
			planet_data.pos_m[2] = planet.star->uPosition.z;
			planet_data.pos_ly[0] = planet.star->position.x;
			planet_data.pos_ly[1] = planet.star->position.y;
			planet_data.pos_ly[2] = planet.star->position.z;
			for(int i=0;i<14;i++) {
				planet_data.veins_point[i] = planet.veins_point[i];
				planet_data.veins_amount[i] = planet.veins_amount[i];
			}
			star_data.planets.push_back(planet_data);
		}
		galaxy_data.stars.push_back(star_data);
	}
	for(StarData& star_data: galaxy_data.stars) {
		PlanetData* last_gas=nullptr;
		for(PlanetData& planet_data: star_data.planets) {
			if(planet_data.is_gas) {
				last_gas = &planet_data;
			} else if(planet_data.singularity & EPlanetSingularity::Satellite) {
				last_gas->moons.push_back(planet_data);
			}
		}
	}
	return galaxy_data;
}

bool check_seed(const SeedStruct& seed,const GalaxyCondition& galaxy_condition,int check_level)
{
	//cout << "start check " << seed.seed_id << " " << (int)seed.star_num << " in level " << check_level << endl;
	//cout << seed.seed_id << " " << (int)seed.star_num << " level1 check start" << endl;
	GalaxyClassSimple galaxy;
	galaxy.CreateStars(seed.seed_id,seed.star_num,resource_rates[seed.resource_index]);
	if(!check_galaxy_level_1(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	if(check_level <= 1)
		return galaxy_condition.valid_state;

	//cout << seed.seed_id << " " << (int)seed.star_num << " level2 check start" << endl;
	galaxy.CreatePlanets(get_need_generate_planet_num(galaxy,galaxy_condition));
	if(!check_galaxy_level_2(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	if(check_level <= 2)
		return galaxy_condition.valid_state;

	//cout << seed.seed_id << " " << (int)seed.star_num << " level3 check start" << endl;
	tag_need_veins_galaxy(galaxy,galaxy_condition);
	galaxy.GenerateUpperVeins();
	if(!check_galaxy_level_3(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	if(check_level <= 3)
		return galaxy_condition.valid_state;

	//cout << seed.seed_id << " " << (int)seed.star_num << " level4 check start" << endl;
	memset(galaxy.veins_point,0,sizeof(galaxy.veins_point));
	memset(galaxy.veins_amount,0,sizeof(galaxy.veins_amount));
	if(!check_galaxy_level_4(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	return galaxy_condition.valid_state;
}
