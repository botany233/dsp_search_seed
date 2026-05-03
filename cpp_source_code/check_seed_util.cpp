#include <array>
#include <memory>
#include <cstdint>
#include <iostream>

#include "defines.hpp"
#include "data_struct.hpp"
#include "astro_class.hpp"

using namespace std;

static bool check_veins(const vector<uint16_t>& need_veins,const uint16_t* has_veins)
{
	for(int i = 0;i < 14;i++) {
		if(need_veins[i] > has_veins[i])
			return false;
	}
	return true;
}

static bool check_veins(const vector<uint16_t>& need_veins,const int* has_veins)
{
	for(int i = 0;i < 14;i++) {
		if(need_veins[i] > has_veins[i])
			return false;
	}
	return true;
}

static bool check_veins(const vector<uint64_t>& need_veins,const uint64_t* has_veins)
{
	for(int i = 0;i < 14;i++) {
		if(need_veins[i] > has_veins[i])
			return false;
	}
	return true;
}

static bool check_planet_veins(const PlanetClassSimple& planet_data,const PlanetCondition& planet_condition)
{
	if((planet_condition.need_veins & planet_data.has_veins) != planet_condition.need_veins)
		return false;
	if(!check_veins(planet_condition.veins_point,planet_data.veins_point))
		return false;
	if(!check_veins(planet_condition.veins_amount,planet_data.veins_amount))
		return false;
	return true;
}

static bool check_star_upper_veins(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if((star_condition.need_veins & star_data.has_veins) != star_condition.need_veins)
		return false;
	if(!check_veins(star_condition.veins_point,star_data.upper_veins_point))
		return false;
	if(!check_veins(star_condition.veins_amount,star_data.upper_veins_amount))
		return false;
	return true;
}

static bool check_star_real_veins(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if(!check_veins(star_condition.veins_point,star_data.real_veins_point))
		return false;
	if(!check_veins(star_condition.veins_amount,star_data.real_veins_amount))
		return false;
	return true;
}

static bool check_galaxy_veins(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	if(!check_veins(galaxy_condition.veins_point,galaxy_data.veins_point))
		return false;
	if(!check_veins(galaxy_condition.veins_amount,galaxy_data.veins_amount))
		return false;
	return true;
}

static uint16_t get_galaxy_veins_mask(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	uint16_t mask = 0;
	for(int i = 0;i < 14;i++) {
		if(galaxy_condition.veins_point[i]>galaxy_data.veins_point[i] || galaxy_condition.veins_amount[i]>galaxy_data.veins_amount[i])
			mask |= 1 << i;
	}
	return mask;
}

static uint16_t get_star_veins_mask(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	uint16_t mask = 0;
	for(int i = 0;i < 14;i++) {
		if(star_condition.veins_point[i]>star_data.real_veins_point[i] || star_condition.veins_amount[i]>star_data.real_veins_amount[i])
			mask |= 1 << i;
	}
	return mask;
}

static bool check_star_level_1(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if(!(star_condition.type & star_data.type_mask))
		return false;
	if(star_condition.distance < star_data.distance)
		return false;
	if(star_condition.dyson_lumino > star_data.luminosity)
		return false;
	return true;
}

bool check_galaxy_level_1(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		int left_satisfy_num = star_condition.satisfy_num;
		for(const StarClassSimple& star_data: galaxy_data.stars) {
			if(check_star_level_1(star_data,star_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	return true;
}

int get_need_generate_planet_index(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	if(galaxy_condition.need_veins || !galaxy_condition.planets.empty())
		return galaxy_data.starCount;
	int result = 0;
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		if(star_condition.need_veins==0 && star_condition.planets.empty())
			continue;
		for(int i=result;i<galaxy_data.starCount;i++) {
			const StarClassSimple& star_data = galaxy_data.stars[i];
			if(check_star_level_1(star_data,star_condition)) {
				result = i;
			}
		}
	}
	return result;
}

static bool check_planet_level_2(const PlanetClassSimple& planet_data,const PlanetCondition& planet_condition)
{
	if(planet_condition.dsp_level > planet_data.dsp_level)
		return false;
	if(!(planet_condition.type & planet_data.type_mask))
		return false;
	if((planet_condition.liquid & planet_data.waterItemId) != planet_condition.liquid)
		return false;
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	if((planet_condition.need_veins & planet_data.has_veins) != planet_condition.need_veins)
		return false;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		int left_satisfy_num = moon_condition.satisfy_num;
		for(const PlanetClassSimple* moon_ptr: planet_data.moons) {
			if(check_planet_level_2(*moon_ptr,moon_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	return true;
}

static bool check_star_level_2(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if(!(star_condition.type & star_data.type_mask))
		return false;
	if(star_condition.distance < star_data.distance)
		return false;
	if(star_condition.dyson_lumino > star_data.luminosity)
		return false;
	if((star_condition.need_veins & star_data.has_veins) != star_condition.need_veins)
		return false;
	for(const PlanetCondition& planet_condition: star_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const PlanetClassSimple& planet_data: star_data.planets) {
			if(check_planet_level_2(planet_data,planet_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	return true;
}

bool check_galaxy_level_2(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		int left_satisfy_num = star_condition.satisfy_num;
		for(const StarClassSimple& star_data: galaxy_data.stars) {
			if(check_star_level_2(star_data,star_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const StarClassSimple& star: galaxy_data.stars) {
			for(const PlanetClassSimple& planet: star.planets) {
				if(check_planet_level_2(planet,planet_condition)) {
					left_satisfy_num--;
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

static bool check_planet_level_3(const PlanetClassSimple& planet_data,const PlanetCondition& planet_condition)
{
	if(planet_condition.dsp_level > planet_data.dsp_level)
		return false;
	if(!(planet_condition.type & planet_data.type_mask))
		return false;
	if((planet_condition.liquid & planet_data.waterItemId) != planet_condition.liquid)
		return false;
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	if(planet_condition.need_veins && !check_planet_veins(planet_data,planet_condition))
		return false;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		int left_satisfy_num = moon_condition.satisfy_num;
		for(const PlanetClassSimple* moon_ptr: planet_data.moons) {
			if(check_planet_level_3(*moon_ptr,moon_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	return true;
}

static bool check_star_level_3(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if(!(star_condition.type & star_data.type_mask))
		return false;
	if(star_condition.distance < star_data.distance)
		return false;
	if(star_condition.dyson_lumino > star_data.luminosity)
		return false;
	if(star_condition.need_veins && !check_star_upper_veins(star_data,star_condition))
		return false;
	for(const PlanetCondition& planet_condition: star_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const PlanetClassSimple& planet_data: star_data.planets) {
			if(check_planet_level_3(planet_data,planet_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	return true;
}

bool check_galaxy_level_3(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	if(galaxy_condition.need_veins && !check_galaxy_veins(galaxy_data,galaxy_condition))
		return false;
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		int left_satisfy_num = star_condition.satisfy_num;
		for(const StarClassSimple& star_data: galaxy_data.stars) {
			if(check_star_level_3(star_data,star_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const StarClassSimple& star_data: galaxy_data.stars) {
			for(const PlanetClassSimple& planet_data: star_data.planets) {
				if(check_planet_level_3(planet_data,planet_condition)) {
					left_satisfy_num--;
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

static bool check_planet_level_4(PlanetClassSimple& planet_data,const PlanetCondition& planet_condition)
{
	if(planet_condition.dsp_level > planet_data.dsp_level)
		return false;
	if(!(planet_condition.type & planet_data.type_mask))
		return false;
	if((planet_condition.liquid & planet_data.waterItemId) != planet_condition.liquid)
		return false;
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		int left_satisfy_num = moon_condition.satisfy_num;
		for(PlanetClassSimple* moon_ptr: planet_data.moons) {
			if(check_planet_level_4(*moon_ptr,moon_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	if(planet_condition.need_veins) {
		if(!planet_data.is_real_veins) {
			if(!check_planet_veins(planet_data,planet_condition))
				return false;
			memset(planet_data.veins_point,0,sizeof(planet_data.veins_point));
			memset(planet_data.veins_amount,0,sizeof(planet_data.veins_amount));
			planet_data.generate_real_veins();
		}
		if(!check_planet_veins(planet_data,planet_condition))
			return false;
	}
	return true;
}

static bool check_star_level_4(StarClassSimple& star_data,const StarCondition& star_condition)
{
	if(!(star_condition.type & star_data.type_mask))
		return false;
	if(star_condition.distance < star_data.distance)
		return false;
	if(star_condition.dyson_lumino > star_data.luminosity)
		return false;
	for(const PlanetCondition& planet_condition: star_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(PlanetClassSimple& planet_data: star_data.planets) {
			if(check_planet_level_4(planet_data,planet_condition)) {
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	if(star_condition.need_veins) {
		if(!check_star_upper_veins(star_data,star_condition))
			return false;
		if(!check_star_real_veins(star_data,star_condition)) {
			for(PlanetClassSimple& planet_data: star_data.planets) {
				if(planet_data.is_real_veins || !(planet_data.has_veins & get_star_veins_mask(star_data,star_condition)))
					continue;
				memset(planet_data.veins_point,0,sizeof(planet_data.veins_point));
				memset(planet_data.veins_amount,0,sizeof(planet_data.veins_amount));
				planet_data.generate_real_veins();
				if(check_star_real_veins(star_data,star_condition))
					goto end_veins_check_label;
			}
			return false;
		}
	}
	end_veins_check_label:
	return true;
}

bool check_galaxy_level_4(GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		int left_satisfy_num = star_condition.satisfy_num;
		for(StarClassSimple& star_data: galaxy_data.stars)
		{
			if(check_star_level_4(star_data,star_condition))
			{
				left_satisfy_num--;
				if(!left_satisfy_num)
					break;
			}
		}
		if(left_satisfy_num)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(StarClassSimple& star_data: galaxy_data.stars) {
			for(PlanetClassSimple& planet_data: star_data.planets) {
				if(check_planet_level_4(planet_data,planet_condition)) {
					left_satisfy_num--;
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
		if(!check_galaxy_veins(galaxy_data,galaxy_condition)) {
			for(StarClassSimple& star_data: galaxy_data.stars) {
				for(PlanetClassSimple& planet_data: star_data.planets) {
					if(planet_data.is_real_veins || !(planet_data.has_veins & get_galaxy_veins_mask(galaxy_data,galaxy_condition)))
						continue;
					memset(planet_data.veins_point,0,sizeof(planet_data.veins_point));
					memset(planet_data.veins_amount,0,sizeof(planet_data.veins_amount));
					planet_data.generate_real_veins();
					if(check_galaxy_veins(galaxy_data,galaxy_condition))
						goto end_veins_check_label;
				}
			}
			return false;
		}
	}
	end_veins_check_label:
	return true;
}
