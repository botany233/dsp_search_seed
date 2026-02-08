#include <array>
#include <cstdint>
#include <iostream>

#include "data_struct.hpp"

using namespace std;

bool check_moon(const PlanetDataSimple& moon_data, const MoonCondition& moon_condition)
{
	if(moon_condition.dsp_level > moon_data.dsp_level)
		return false;
	if(moon_condition.type && moon_condition.type != moon_data.type &&
		!(moon_condition.type == 23 && moon_data.type == 22))
		return false;
	if((moon_condition.liquid & moon_data.liquid) != moon_condition.liquid)
		return false;
	if((moon_condition.singularity & moon_data.singularity) != moon_condition.singularity)
		return false;
	if(moon_condition.need_veins) {
		for(int i = 0; i < 14; i++) {
			if(moon_condition.veins_group[i] > moon_data.veins_group[i] || 
               moon_condition.veins_point[i] > moon_data.veins_point[i])
				return false;
		}
	}
	return true;
}

bool check_planet(const StarDataSimple& star_data, int planet_index, const PlanetCondition& planet_condition)
{
	const PlanetDataSimple& planet_data = star_data.planets[planet_index];
	if(planet_condition.dsp_level > planet_data.dsp_level)
		return false;
	if(planet_condition.type && planet_condition.type != planet_data.type &&
		!(planet_condition.type == 23 && planet_data.type == 22))
		return false;
	if((planet_condition.liquid & planet_data.liquid) != planet_condition.liquid)
		return false;
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	if(planet_condition.need_veins) {
		for(int i = 0;i < 14;i++) {
			if(planet_condition.veins_group[i] > planet_data.veins_group[i] || planet_condition.veins_point[i] > planet_data.veins_point[i])
				return false;
		}
	}
	if (!planet_condition.moons.empty()) {
        for (const MoonCondition& moon_cond : planet_condition.moons) {
            int left_satisfy_num = moon_cond.satisfy_num;
            for (int m_idx : planet_data.moon_indexes) {
                if (check_moon(star_data.planets[m_idx], moon_cond)) {
                    left_satisfy_num--;
                    if (left_satisfy_num <= 0) break;
                }
            }
            if (left_satisfy_num > 0) return false;
        }
    }
	return true;
}

bool check_star(const StarDataSimple& star_data,const StarCondition& star_condition)
{
	if(star_condition.type && star_condition.type != star_data.type)
		return false;
	if(star_condition.distance < star_data.distance)
		return false;
	if(star_condition.dyson_lumino > star_data.dyson_lumino)
		return false;
	for(const PlanetCondition& planet_condition: star_condition.planets)
	{
		int left_satisfy_num = planet_condition.satisfy_num;
		for(int i = 0; i < star_data.planets.size(); i++) {
			if(check_planet(star_data, i, planet_condition))
			{
				left_satisfy_num -= 1;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	if(star_condition.need_veins) {
		for(int i = 0;i < 14;i++) {
			if(star_condition.veins_group[i] > star_data.veins_group[i] || star_condition.veins_point[i] > star_data.veins_point[i])
				return false;
		}
	}
	return true;
}

bool check_galaxy(const GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		int left_satisfy_num = star_condition.satisfy_num;
		for(const StarDataSimple& star_data: galaxy_data.stars)
		{
			if(check_star(star_data,star_condition))
			{
				left_satisfy_num -= 1;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets)
	{
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const StarDataSimple& star_data: galaxy_data.stars)
		{
			for(int i = 0; i < star_data.planets.size(); ++i)
			{
				if(check_planet(star_data, i, planet_condition))
				{
					left_satisfy_num -= 1;
					if(!left_satisfy_num)
						goto end_check_label;
				}
			}
		}
		end_check_label:
		if(left_satisfy_num)
			return false;
	}
	if(galaxy_condition.need_veins) {
		for(int i = 0;i < 14;i++) {
			if(galaxy_condition.veins_group[i] > galaxy_data.veins_group[i] || galaxy_condition.veins_point[i] > galaxy_data.veins_point[i])
				return false;
		}
	}
	return true;
}

bool check_moon_quick(const PlanetDataSimple& moon_data, const MoonCondition& moon_condition)
{
	if(moon_condition.dsp_level > moon_data.dsp_level)
		return false;
	if(moon_condition.type && moon_condition.type != moon_data.type &&
		!(moon_condition.type == 23 && moon_data.type == 22))
		return false;
	if((moon_condition.liquid & moon_data.liquid) != moon_condition.liquid)
		return false;
	if((moon_condition.singularity & moon_data.singularity) != moon_condition.singularity)
		return false;
	return true;
}

bool check_planet_quick(const StarDataSimple& star_data, int planet_index, const PlanetCondition& planet_condition)
{
    const PlanetDataSimple& planet_data = star_data.planets[planet_index];

	if(planet_condition.dsp_level > planet_data.dsp_level)
		return false;
	if(planet_condition.type && planet_condition.type != planet_data.type &&
		!(planet_condition.type == 23 && planet_data.type == 22))
		return false;
	if((planet_condition.liquid & planet_data.liquid) != planet_condition.liquid)
		return false;
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;

    if (!planet_condition.moons.empty()) {
        for (const MoonCondition& moon_cond : planet_condition.moons) {
            int left_satisfy_num = moon_cond.satisfy_num;
            for (int moon_idx : planet_data.moon_indexes) {
                const PlanetDataSimple& moon_data = star_data.planets[moon_idx];
                if (check_moon_quick(moon_data, moon_cond)) {
                    left_satisfy_num -= 1;
                    if (!left_satisfy_num) break;
                }
            }
            if (left_satisfy_num > 0)
                return false;
        }
    }

	return true;
}

bool check_star_quick(const StarDataSimple& star_data,const StarCondition& star_condition)
{
	if(star_condition.type && star_condition.type != star_data.type)
		return false;
	if(star_condition.distance < star_data.distance)
		return false;
	if(star_condition.dyson_lumino > star_data.dyson_lumino)
		return false;
	for(const PlanetCondition& planet_condition: star_condition.planets)
	{
		int left_satisfy_num = planet_condition.satisfy_num;
		for(int i = 0; i < star_data.planets.size(); ++i)
		{
			if(check_planet_quick(star_data, i, planet_condition))
			{
				left_satisfy_num -= 1;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	return true;
}

bool check_galaxy_quick(const GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		int left_satisfy_num = star_condition.satisfy_num;
		for(const StarDataSimple& star_data: galaxy_data.stars)
		{
			if(check_star_quick(star_data,star_condition))
			{
				left_satisfy_num -= 1;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets)
	{
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const StarDataSimple& star_data: galaxy_data.stars)
		{
			for(int i = 0; i < star_data.planets.size(); ++i)
			{
				if(check_planet_quick(star_data, i, planet_condition))
				{
					left_satisfy_num -= 1;
					if(!left_satisfy_num)
						goto end_check_label;
				}
			}
		}
		end_check_label:
		if(left_satisfy_num)
			return false;
	}
	return true;
}

bool planet_and_moons_no_veins(const PlanetConditionSimple& pc) {
    if (pc.need_veins) return false;
    for (const auto& moon : pc.moons) {
        if (moon.need_veins) return false;
    }
    return true;
}

void del_empty_galaxy_condition(GalaxyConditionSimple& galaxy_condition) {
	int index1 = 0;
	while(index1 < galaxy_condition.planets.size()) {
		if(!planet_and_moons_no_veins(galaxy_condition.planets[index1])) {
			index1++;
		} else {
			galaxy_condition.planets.erase(galaxy_condition.planets.begin() + index1);
		}
	}
	int index2 = 0;
	while(index2 < galaxy_condition.stars.size()) {
		StarConditionSimple& star_condition = galaxy_condition.stars[index2];
		int index3 = 0;
		while(index3 < star_condition.planets.size()) {
			if(!planet_and_moons_no_veins(star_condition.planets[index3])) {
				index3++;
			} else {
				star_condition.planets.erase(star_condition.planets.begin() + index3);
				for(StarIndexStruct& si_struct: star_condition.star_indexes) {
					si_struct.satisfy_planets.erase(si_struct.satisfy_planets.begin() + index3);
				}
			}
		}
		if(star_condition.need_veins || star_condition.planets.size()) {
			index2++;
		} else {
			galaxy_condition.stars.erase(galaxy_condition.stars.begin() + index2);
		}
	}
}

void get_galaxy_condition_struct(const GalaxyDataSimple& galaxy_data,const GalaxyCondition& galaxy_condition,GalaxyConditionSimple& galaxy_condition_simple)
{
	for(int sc_index=0;sc_index<galaxy_condition.stars.size();sc_index++) {
		const StarCondition& star_condition = galaxy_condition.stars[sc_index];
		StarConditionSimple& star_condition_simple = galaxy_condition_simple.stars[sc_index];
		for(const StarDataSimple& star_data: galaxy_data.stars) {
			if(check_star(star_data,star_condition)) {
				StarIndexStruct star_index_struct = StarIndexStruct();
				star_index_struct.star_index = star_data.index;
				for(int pc_index=0;pc_index<star_condition_simple.planets.size();pc_index++) {
					const PlanetCondition& planet_condition = star_condition.planets[pc_index];
					PlanetConditionSimple& planet_condition_simple = star_condition_simple.planets[pc_index];
					vector<int> temp_vector = vector<int>();
					for(const PlanetDataSimple& planet_data: star_data.planets) {
						if(check_planet(star_data, planet_data.index, planet_condition)) {
							temp_vector.push_back(planet_data.index);
						}
					}
					star_index_struct.satisfy_planets.push_back(temp_vector);
				}
				star_condition_simple.star_indexes.push_back(star_index_struct);
			}
		}
	}
	for(int pc_index=0;pc_index<galaxy_condition.planets.size();pc_index++) {
		const PlanetCondition& planet_condition = galaxy_condition.planets[pc_index];
		PlanetConditionSimple& planet_condition_struct = galaxy_condition_simple.planets[pc_index];
		for(const StarDataSimple& star_data: galaxy_data.stars) {
			for(const PlanetDataSimple& planet_data: star_data.planets) {
				if(check_planet(star_data, planet_data.index, planet_condition)) {
					PlanetIndexStruct planet_index_struct = PlanetIndexStruct();
					planet_index_struct.star_index = star_data.index;
					planet_index_struct.planet_index = planet_data.index;
					planet_condition_struct.planet_indexes.push_back(planet_index_struct);
				}
			}
		}
	}
	del_empty_galaxy_condition(galaxy_condition_simple);
}

MoonConditionSimple init_moon_condition_simple(const MoonCondition& moon_condition)
{
	MoonConditionSimple moon_condition_simple = MoonConditionSimple();
	moon_condition_simple.satisfy_num = moon_condition.satisfy_num;
	if(moon_condition.need_veins) {
		moon_condition_simple.need_veins = moon_condition.need_veins;
		for(int i=0;i<14;i++) {
			moon_condition_simple.veins_group[i] = moon_condition.veins_group[i];
			moon_condition_simple.veins_point[i] = moon_condition.veins_point[i];
		}
	}
	return moon_condition_simple;
}

PlanetConditionSimple init_planet_condition_simple(const PlanetCondition& planet_condition)
{
	PlanetConditionSimple planet_condition_simple = PlanetConditionSimple();
	planet_condition_simple.satisfy_num = planet_condition.satisfy_num;
	if(planet_condition.need_veins) {
		planet_condition_simple.need_veins = planet_condition.need_veins;
		for(int i=0;i<14;i++) {
			planet_condition_simple.veins_group[i] = planet_condition.veins_group[i];
			planet_condition_simple.veins_point[i] = planet_condition.veins_point[i];
		}
	}
	for(const MoonCondition& moon_condition: planet_condition.moons) {
		planet_condition_simple.moons.push_back(init_moon_condition_simple(moon_condition));
	}
	return planet_condition_simple;
}

StarConditionSimple init_star_condition_struct(const StarCondition& star_condition)
{
	StarConditionSimple star_condition_simple = StarConditionSimple();
	star_condition_simple.satisfy_num = star_condition.satisfy_num;
	if(star_condition.need_veins) {
		star_condition_simple.need_veins = star_condition.need_veins;
		for(int i=0;i<14;i++) {
			star_condition_simple.veins_group[i] = star_condition.veins_group[i];
			star_condition_simple.veins_point[i] = star_condition.veins_point[i];
		}
	}
	for(const PlanetCondition& planet_condition: star_condition.planets) {
		star_condition_simple.planets.push_back(init_planet_condition_simple(planet_condition));
	}
	return star_condition_simple;
}

GalaxyConditionSimple init_galaxy_condition_struct(const GalaxyCondition& galaxy_condition)
{
	GalaxyConditionSimple galaxy_condition_simple = GalaxyConditionSimple();
	if(galaxy_condition.need_veins) {
		galaxy_condition_simple.need_veins = galaxy_condition.need_veins;
		for(int i=0;i<14;i++) {
			galaxy_condition_simple.veins_group[i] = galaxy_condition.veins_group[i];
			galaxy_condition_simple.veins_point[i] = galaxy_condition.veins_point[i];
		}
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		galaxy_condition_simple.planets.push_back(init_planet_condition_simple(planet_condition));
	}
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		galaxy_condition_simple.stars.push_back(init_star_condition_struct(star_condition));
	}
	return galaxy_condition_simple;
}
