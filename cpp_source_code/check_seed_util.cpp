#include "json.hpp"
#include "data_struct.hpp"

using namespace nlohmann;

bool check_planet(PlanetStructSimple& planet_data,const json& planet_condition,const bool update_flag)
{
	if(planet_condition.contains("dsp_level") && planet_condition["dsp_level"] > planet_data.dsp_level)
		return false;
	if(planet_condition.contains("type") && planet_condition["type"] != planet_data.type &&
		!(planet_condition["type"] == "气态巨星" && planet_data.type == "高产气巨"))
		return false;
	if(planet_condition.contains("liquid") && planet_condition["liquid"] != planet_data.liquid)
		return false;
	if(planet_condition.contains("singularity"))
	{
		bool tag = true;
		for(const string& planet_singularity: planet_data.singularity)
		{
			if(planet_condition["singularity"] == planet_singularity)
			{
				tag = false;
				break;
			}
		}
		if(tag)
			return false;
	}
	if(planet_condition.contains("veins"))
	{
		if(planet_data.is_gas)
			return false;
		else
		{
			for(int i = 0;i < 14;i++)
			{
				if(planet_condition["veins"][i] > planet_data.veins[i])
					return false;
			}
		}
	}
	if(planet_condition.contains("veins_point"))
	{
		if(planet_data.is_gas)
			return false;
		else
		{
			for(int i = 0;i < 14;i++)
			{
				if(planet_condition["veins_point"][i] > planet_data.veins_point[i])
					return false;
			}
		}
	}
	if(update_flag && planet_condition.contains("need_veins"))
		planet_data.need_veins |= planet_condition["need_veins"];
	return true;
}

bool check_star(StarStructSimple& star_data,const json& star_condition,const bool update_flag)
{
	if(star_condition.contains("type") && star_condition["type"] != star_data.type)
		return false;
	if(star_condition.contains("distance") && star_condition["distance"] < star_data.distance)
		return false;
	if(star_condition.contains("dyson_lumino") && star_condition["dyson_lumino"] > star_data.dyson_lumino)
		return false;
	if(star_condition.contains("planets"))
	{
		for(const json& planet_condition: star_condition["planets"])
		{
			int left_satisfy_num = planet_condition["satisfy_num"];
			for(PlanetStructSimple& planet_data: star_data.planets)
			{
				if(check_planet(planet_data,planet_condition,update_flag))
				{
					left_satisfy_num -= 1;
					if(!left_satisfy_num)
						break;
				}
			}
			if(left_satisfy_num)
				return false;
		}
	}
	if(star_condition.contains("veins"))
	{
		for(int i = 0;i < 14;i++)
		{
			if(star_condition["veins"][i] > star_data.veins[i])
				return false;
		}
	}
	if(star_condition.contains("veins_point"))
	{
		for(int i = 0;i < 14;i++)
		{
			if(star_condition["veins_point"][i] > star_data.veins_point[i])
				return false;
		}
	}
	if(update_flag && star_condition.contains("need_veins"))
		star_data.need_veins |= star_condition["need_veins"];
	return true;
}

bool check_galaxy(GalaxyStructSimple& galaxy_data,const json& galaxy_condition,const bool update_flag)
{
	if(galaxy_condition.contains("stars"))
	{
		for(const json& star_condition: galaxy_condition["stars"])
		{
			int left_satisfy_num = star_condition["satisfy_num"];
			for(StarStructSimple& star_data: galaxy_data.stars)
			{
				if(check_star(star_data,star_condition,update_flag))
				{
					left_satisfy_num -= 1;
					if(!left_satisfy_num)
						break;
				}
			}
			if(left_satisfy_num)
				return false;
		}
	}
	if(galaxy_condition.contains("planets"))
	{
		for(const json& planet_condition: galaxy_condition["planets"])
		{
			int left_satisfy_num = planet_condition["satisfy_num"];
			for(StarStructSimple& star_data: galaxy_data.stars)
			{
				for(PlanetStructSimple& planet_data: star_data.planets)
				{
					if(check_planet(planet_data,planet_condition,update_flag))
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
	}
	if(galaxy_condition.contains("veins"))
	{
		for(int i = 0;i < 14;i++)
		{
			if(galaxy_condition["veins"][i] > galaxy_data.veins[i])
				return false;
		}
	}
	if(galaxy_condition.contains("veins_point"))
	{
		for(int i = 0;i < 14;i++)
		{
			if(galaxy_condition["veins_point"][i] > galaxy_data.veins_point[i])
				return false;
		}
	}
	if(update_flag && galaxy_condition.contains("need_veins"))
		galaxy_data.need_veins |= galaxy_condition["need_veins"];
	return true;
}

bool check_planet_quick(const PlanetStructSimple& planet_data,const json& planet_condition)
{
	if(planet_condition.contains("dsp_level") && planet_condition["dsp_level"] > planet_data.dsp_level)
		return false;
	if(planet_condition.contains("type") && planet_condition["type"] != planet_data.type &&
		!(planet_condition["type"] == "气态巨星" && planet_data.type == "高产气巨"))
		return false;
	if(planet_condition.contains("liquid") && planet_condition["liquid"] != planet_data.liquid)
		return false;
	if(planet_condition.contains("singularity"))
	{
		bool tag = true;
		for(const string& planet_singularity: planet_data.singularity)
		{
			if(planet_condition["singularity"] == planet_singularity)
			{
				tag = false;
				break;
			}
		}
		if(tag)
			return false;
	}
	return true;
}

bool check_star_quick(const StarStructSimple& star_data,const json& star_condition)
{
	if(star_condition.contains("type") && star_condition["type"] != star_data.type)
		return false;
	if(star_condition.contains("distance") && star_condition["distance"] < star_data.distance)
		return false;
	if(star_condition.contains("dyson_lumino") && star_condition["dyson_lumino"] > star_data.dyson_lumino)
		return false;
	if(star_condition.contains("planets"))
	{
		for(const json& planet_condition: star_condition["planets"])
		{
			int left_satisfy_num = planet_condition["satisfy_num"];
			for(const PlanetStructSimple& planet_data: star_data.planets)
			{
				if(check_planet_quick(planet_data,planet_condition))
				{
					left_satisfy_num -= 1;
					if(!left_satisfy_num)
						break;
				}
			}
			if(left_satisfy_num)
				return false;
		}
	}
	return true;
}

bool check_galaxy_quick(const GalaxyStructSimple& galaxy_data,const json& galaxy_condition)
{
	if(galaxy_condition.contains("stars"))
	{
		for(const json& star_condition: galaxy_condition["stars"])
		{
			int left_satisfy_num = star_condition["satisfy_num"];
			for(const StarStructSimple& star_data: galaxy_data.stars)
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
	}
	if(galaxy_condition.contains("planets"))
	{
		for(const json& planet_condition: galaxy_condition["planets"])
		{
			int left_satisfy_num = planet_condition["satisfy_num"];
			for(const StarStructSimple& star_data: galaxy_data.stars)
			{
				if(!left_satisfy_num)
					break;
				for(const PlanetStructSimple& planet_data: star_data.planets)
				{
					if(check_planet_quick(planet_data,planet_condition))
					{
						left_satisfy_num -= 1;
						if(!left_satisfy_num)
							break;
					}
				}
			}
			if(left_satisfy_num)
				return false;
		}
	}
	return true;
}
