#include <array>
#include <memory>
#include <cstdint>
#include <variant>
#include <iostream>

#include "defines.hpp"
#include "data_struct.hpp"
#include "astro_class.hpp"
#include "max_flow.hpp"
#include "VectorLF3.hpp"

using namespace std;

static bool check_bond_position(vector<VectorLF3> pos1,vector<VectorLF3> pos2,const BondCondition bond_condition) {
	int limit1 = std::visit([](const auto& c) { return c.satisfy_num; },bond_condition.con1);
	int limit2 = std::visit([](const auto& c) { return c.satisfy_num; },bond_condition.con2);
	if(pos1.size()*limit1 < bond_condition.satisfy_num || pos2.size()*limit2 < bond_condition.satisfy_num)
		return false;
	int n1 = pos1.size();
	int n2 = pos2.size();
	int n = n1 + n2 + 2;
	MaxFlowGraph graph(n);
	for(int i = 1; i < n1+1; i++)
		graph.add_edge(0,i,limit1);
	for(int i = n1 +1; i < n - 1; i++)
		graph.add_edge(i,n-1,limit2);

	for(int x=0;x<n1;x++) {
		for(int y=0;y<n2;y++) {
			if((pos1[x]-pos2[y]).magnitude() <= bond_condition.distance) {
				graph.add_edge(x+1,n1+y+1);
			}
		}
	}
	return graph.flow(0,n-1,bond_condition.satisfy_num);
}

template<typename T>
static bool check_veins(const array<T,14>& need_veins,const T* has_veins)
{
	for(int i = 0; i < need_veins.size(); i++) {
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
	if(planet_condition.need_veins_amount && !check_veins(planet_condition.veins_amount,planet_data.veins_amount))
		return false;
	return true;
}

static bool check_star_upper_veins(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if((star_condition.need_veins & star_data.has_veins) != star_condition.need_veins)
		return false;
	if(!check_veins(star_condition.veins_point,star_data.upper_veins_point))
		return false;
	if(star_condition.need_veins_amount && !check_veins(star_condition.veins_amount,star_data.upper_veins_amount))
		return false;
	return true;
}

static bool check_star_real_veins(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	if(!check_veins(star_condition.veins_point,star_data.real_veins_point))
		return false;
	if(star_condition.need_veins_amount && !check_veins(star_condition.veins_amount,star_data.real_veins_amount))
		return false;
	return true;
}

static bool check_galaxy_veins(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	if(!check_veins(galaxy_condition.veins_point,galaxy_data.veins_point))
		return false;
	if(galaxy_condition.need_veins_amount && !check_veins(galaxy_condition.veins_amount,galaxy_data.veins_amount))
		return false;
	return true;
}

static uint16_t get_galaxy_veins_mask(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	uint16_t mask = 0;
	for(int i = 0;i < galaxy_condition.veins_point.size();i++)
		mask |= (uint16_t)(galaxy_condition.veins_point[i]>galaxy_data.veins_point[i]) << i;
	if(galaxy_condition.need_veins_amount)
		for(int i = 0;i < galaxy_condition.veins_amount.size();i++)
			mask |= (uint16_t)(galaxy_condition.veins_amount[i]>galaxy_data.veins_amount[i]) << i;
	return mask;
}

static uint16_t get_star_veins_mask(const StarClassSimple& star_data,const StarCondition& star_condition)
{
	uint16_t mask = 0;
	for(int i = 0;i < star_condition.veins_point.size();i++)
		mask |= (uint16_t)(star_condition.veins_point[i]>star_data.real_veins_point[i]) << i;
	if(star_condition.need_veins_amount)
		for(int i = 0;i < star_condition.veins_amount.size();i++)
			mask |= (uint16_t)(star_condition.veins_amount[i]>star_data.real_veins_amount[i]) << i;
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
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	return true;
}

int get_need_generate_planet_num(const GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition)
{
	if(galaxy_condition.need_veins || !galaxy_condition.planets.empty() || !galaxy_condition.bonds.empty())
		return galaxy_data.starCount;
	int result = 0;
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		if(star_condition.need_veins==0 && star_condition.planets.empty())
			continue;
		for(int i=result;i<galaxy_data.starCount;i++) {
			const StarClassSimple& star_data = galaxy_data.stars[i];
			if(check_star_level_1(star_data,star_condition)) {
				result = i+1;
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
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	if((planet_condition.need_veins & planet_data.has_veins) != planet_condition.need_veins)
		return false;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		int left_satisfy_num = moon_condition.satisfy_num;
		for(const PlanetClassSimple& moon: planet_data.moons) {
			if(check_planet_level_2(moon,moon_condition)) {
				left_satisfy_num--;
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
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
				if(left_satisfy_num <= 0)
					break;
			}
		}
		if(left_satisfy_num > 0)
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
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const PlanetClassSimple& planet: galaxy_data.planets) {
			if(check_planet_level_2(planet,planet_condition)) {
				left_satisfy_num--;
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	for(const BondCondition& bond_condition: galaxy_condition.bonds) {
		vector<VectorLF3> pos1,pos2;
		pos1.reserve(galaxy_data.planetCount);
		pos2.reserve(galaxy_data.planetCount);
		if(holds_alternative<PlanetCondition>(bond_condition.con1)) {
			for(const PlanetClassSimple& planet_data : galaxy_data.planets)
				if(check_planet_level_2(planet_data,get<PlanetCondition>(bond_condition.con1)))
					pos1.push_back(planet_data.star->position);
		} else {
			for(const StarClassSimple& star_data : galaxy_data.stars)
				if(check_star_level_2(star_data,get<StarCondition>(bond_condition.con1)))
					pos1.push_back(star_data.position);
		}
		if(holds_alternative<PlanetCondition>(bond_condition.con2)) {
			for(const PlanetClassSimple& planet_data : galaxy_data.planets)
				if(check_planet_level_2(planet_data,get<PlanetCondition>(bond_condition.con2)))
					pos2.push_back(planet_data.star->position);
		} else {
			for(const StarClassSimple& star_data : galaxy_data.stars)
				if(check_star_level_2(star_data,get<StarCondition>(bond_condition.con2)))
					pos2.push_back(star_data.position);
		}
		if(!check_bond_position(pos1,pos2,bond_condition))
			return false;
	}
	return true;
}

void tag_need_veins_planet(PlanetClassSimple& planet_data,const PlanetCondition& planet_condition) {
	if(!check_planet_level_2(planet_data,planet_condition))
		return;
	planet_data.need_generate_veins = true;
	if(planet_condition.need_veins_amount)
		planet_data.need_generate_veins_amount = true;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		for(PlanetClassSimple& moon: planet_data.moons) {
			tag_need_veins_planet(moon,moon_condition);
		}
	}
}

void tag_need_veins_star(StarClassSimple& star_data,const StarCondition& star_condition) {
	if(!check_star_level_2(star_data,star_condition))
		return;
	if(star_condition.need_veins) {
		for(PlanetClassSimple& planet_data: star_data.planets) {
			if(star_condition.need_veins & planet_data.has_veins) {
				planet_data.need_generate_veins = true;
				if(star_condition.need_veins_amount)
					planet_data.need_generate_veins_amount = true;
			}
		}
	}
	for(const PlanetCondition& planet_condition: star_condition.planets) {
		for(PlanetClassSimple& planet_data: star_data.planets) {
			tag_need_veins_planet(planet_data,planet_condition);
		}
	}
}

void tag_need_veins_galaxy(GalaxyClassSimple& galaxy_data,const GalaxyCondition& galaxy_condition) {
	if(galaxy_condition.need_veins) {
		for(PlanetClassSimple& planet_data: galaxy_data.planets) {
			if(galaxy_condition.need_veins & planet_data.has_veins) {
				planet_data.need_generate_veins = true;
				if(galaxy_condition.need_veins_amount)
					planet_data.need_generate_veins_amount = true;
			}
		}
	}
	for(const StarCondition& star_condition: galaxy_condition.stars) {
		for(StarClassSimple& star_data: galaxy_data.stars) {
			tag_need_veins_star(star_data,star_condition);
		}
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		for(PlanetClassSimple& planet_data: galaxy_data.planets)
			tag_need_veins_planet(planet_data,planet_condition);
	}
	for(const BondCondition& bond_condition: galaxy_condition.bonds) {
		if(holds_alternative<PlanetCondition>(bond_condition.con1)) {
			for(PlanetClassSimple& planet_data: galaxy_data.planets) {
				tag_need_veins_planet(planet_data,get<PlanetCondition>(bond_condition.con1));
			}
		} else {
			for(StarClassSimple& star_data: galaxy_data.stars) {
				tag_need_veins_star(star_data,get<StarCondition>(bond_condition.con1));
			}
		}
		if(holds_alternative<PlanetCondition>(bond_condition.con2)) {
			for(PlanetClassSimple& planet_data: galaxy_data.planets) {
				tag_need_veins_planet(planet_data,get<PlanetCondition>(bond_condition.con2));
			}
		} else {
			for(StarClassSimple& star_data: galaxy_data.stars) {
				tag_need_veins_star(star_data,get<StarCondition>(bond_condition.con2));
			}
		}
	}
}

static bool check_planet_level_3(const PlanetClassSimple& planet_data,const PlanetCondition& planet_condition)
{
	if(planet_condition.dsp_level > planet_data.dsp_level)
		return false;
	if(!(planet_condition.type & planet_data.type_mask))
		return false;
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	if(planet_condition.need_veins && !check_planet_veins(planet_data,planet_condition))
		return false;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		int left_satisfy_num = moon_condition.satisfy_num;
		for(const PlanetClassSimple& moon: planet_data.moons) {
			if(check_planet_level_3(moon,moon_condition)) {
				left_satisfy_num--;
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
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
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
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
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(const PlanetClassSimple& planet_data: galaxy_data.planets) {
			if(check_planet_level_3(planet_data,planet_condition)) {
				left_satisfy_num--;
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	for(const BondCondition& bond_condition: galaxy_condition.bonds) {
		vector<VectorLF3> pos1,pos2;
		pos1.reserve(galaxy_data.planetCount);
		pos2.reserve(galaxy_data.planetCount);
		if(holds_alternative<PlanetCondition>(bond_condition.con1)) {
			for(const PlanetClassSimple& planet_data : galaxy_data.planets)
				if(check_planet_level_3(planet_data,get<PlanetCondition>(bond_condition.con1)))
					pos1.push_back(planet_data.star->position);
		} else {
			for(const StarClassSimple& star_data : galaxy_data.stars)
				if(check_star_level_3(star_data,get<StarCondition>(bond_condition.con1)))
					pos1.push_back(star_data.position);
		}
		if(holds_alternative<PlanetCondition>(bond_condition.con2)) {
			for(const PlanetClassSimple& planet_data : galaxy_data.planets)
				if(check_planet_level_3(planet_data,get<PlanetCondition>(bond_condition.con2)))
					pos2.push_back(planet_data.star->position);
		} else {
			for(const StarClassSimple& star_data : galaxy_data.stars)
				if(check_star_level_3(star_data,get<StarCondition>(bond_condition.con2)))
					pos2.push_back(star_data.position);
		}
		if(!check_bond_position(pos1,pos2,bond_condition))
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
	if((planet_condition.singularity & planet_data.singularity) != planet_condition.singularity)
		return false;
	for(const PlanetCondition& moon_condition: planet_condition.moons) {
		int left_satisfy_num = moon_condition.satisfy_num;
		for(PlanetClassSimple& moon: planet_data.moons) {
			if(check_planet_level_4(moon,moon_condition)) {
				left_satisfy_num--;
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
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
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
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
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	for(const PlanetCondition& planet_condition: galaxy_condition.planets) {
		int left_satisfy_num = planet_condition.satisfy_num;
		for(PlanetClassSimple& planet_data: galaxy_data.planets) {
			if(check_planet_level_4(planet_data,planet_condition)) {
				left_satisfy_num--;
				if(left_satisfy_num<=0)
					break;
			}
		}
		if(left_satisfy_num>0)
			return false;
	}
	if(galaxy_condition.need_veins) {
		if(!check_galaxy_veins(galaxy_data,galaxy_condition)) {
			for(PlanetClassSimple& planet_data: galaxy_data.planets) {
				if(planet_data.is_real_veins || !(planet_data.has_veins & get_galaxy_veins_mask(galaxy_data,galaxy_condition)))
					continue;
				memset(planet_data.veins_point,0,sizeof(planet_data.veins_point));
				memset(planet_data.veins_amount,0,sizeof(planet_data.veins_amount));
				planet_data.generate_real_veins();
				if(check_galaxy_veins(galaxy_data,galaxy_condition))
					goto end_veins_check_label;
			}
			return false;
		}
	}
	end_veins_check_label:
	for(const BondCondition& bond_condition: galaxy_condition.bonds) {
		vector<VectorLF3> pos1,pos2;
		pos1.reserve(galaxy_data.planetCount);
		pos2.reserve(galaxy_data.planetCount);
		if(holds_alternative<PlanetCondition>(bond_condition.con1)) {
			for(PlanetClassSimple& planet_data : galaxy_data.planets)
				if(check_planet_level_4(planet_data,get<PlanetCondition>(bond_condition.con1)))
					pos1.push_back(planet_data.star->position);
		} else {
			for(StarClassSimple& star_data : galaxy_data.stars)
				if(check_star_level_4(star_data,get<StarCondition>(bond_condition.con1)))
					pos1.push_back(star_data.position);
		}
		if(holds_alternative<PlanetCondition>(bond_condition.con2)) {
			for(PlanetClassSimple& planet_data : galaxy_data.planets)
				if(check_planet_level_4(planet_data,get<PlanetCondition>(bond_condition.con2)))
					pos2.push_back(planet_data.star->position);
		} else {
			for(StarClassSimple& star_data : galaxy_data.stars)
				if(check_star_level_4(star_data,get<StarCondition>(bond_condition.con2)))
					pos2.push_back(star_data.position);
		}
		if(!check_bond_position(pos1,pos2,bond_condition))
			return false;
	}
	return true;
}
