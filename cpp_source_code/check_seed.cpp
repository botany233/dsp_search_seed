#include "json.hpp"
#include "DSPGen.hpp"

#include "check_seed_util.hpp"
#include "data_struct.hpp"

using namespace nlohmann;
using namespace std;

const int planet_id_to_type[25] = {
	0, 22, 22, 1, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	21, 17, 18, 19, 20
};

json get_galaxy_data(int seed,int star_num)
{
	json galaxy_data = {};
	UniverseGen g;
	g.CreateGalaxy(seed,star_num,0);
	galaxy_data["seed"] = seed;
	galaxy_data["star_count"] = star_num;
	galaxy_data["stars"] = json::array();
	int galaxy_res[14] = {0};
	int planet_type_nums[23] = {0};
	for(StarData star : g.stars)
	{
		json star_data = {};
		star_data["type"] = star.typeString();
		star_data["name"] = star.name;
		int star_res[14] = {0};
		float distanc = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data["distance"] = distanc;
		star_data["dyson_lumino"] = star.dysonLumino();
		star_data["dyson_radius"] = star.dysonRadius;
		star_data["seed"] = star.seed;
		star_data["planets"] = json::array();
		for(PlanetData planet : star.planets)
		{
			json planet_data = {};
			planet_data["id"] = planet.id;
			planet_data["seed"] = planet.seed;
			planet_data["type"] = LDB.Select(planet.theme).DisplayName;
			planet_data["singularity"] = planet.GetPlanetSingularityVector();
			planet_data["lumino"] = planet.luminosity;
			planet_data["wind"] = planet.windStrength;
			planet_data["radius"] = planet.orbitRadius;
			planet_data["is_in_dsp"] = star.dysonRadius > planet.maxorbitRadius * 0.6770833f;
			if(planet.waterItemId == 1000)
				planet_data["liquid"] = "水";
			else if(planet.waterItemId == 1116)
				planet_data["liquid"] = "硫酸";
			else
				planet_data["liquid"] = "无";
			if(planet.gasItems.size())
			{
				planet_data["is_gas"] = true;
				planet_data["veins"] = {{"氢",0},{"重氢",0},{"可燃冰",0}};
				for(int j = 0; j < 2; j++)
				{
					if(planet.gasItems[j] == 1120)
						planet_data["veins"]["氢"] = planet.gasSpeeds[j];
					else if(planet.gasItems[j] == 1121)
						planet_data["veins"]["重氢"] = planet.gasSpeeds[j];
					else if(planet.gasItems[j] == 1011)
						planet_data["veins"]["可燃冰"] = planet.gasSpeeds[j];
				}
			} else
			{
				planet_data["is_gas"] = false;
				int planet_res[14] = {0};
				g.MyGenerateVeins(star,planet,planet_res);
				planet_data["veins"] = planet_res;
				for(int i = 0; i < 14; i++)
					star_res[i] += planet_res[i];
			}
			star_data["planets"].push_back(planet_data);
			planet_type_nums[planet_id_to_type[planet.theme - 1]] += 1;
		}
		star_data["veins"] = star_res;
		for(int i = 0; i < 14; i++)
			galaxy_res[i] += star_res[i];
		galaxy_data["stars"].push_back(star_data);
	}
	galaxy_data["veins"] = galaxy_res;
	galaxy_data["planet_type_nums"] = planet_type_nums;
	//return galaxy_json.dump(4);
	//return galaxy_json.dump(4, ' ', false, json::error_handler_t::replace);
	return galaxy_data;
}

bool check_seed(int seed,int star_num,const json& galaxy_condition,const json& galaxy_condition_simple,bool check_no_veins)
{
	GalaxyStruct galaxy_data;
	UniverseGen g;
	g.CreateGalaxy(seed,star_num,1);
	for(StarData star : g.stars)
	{
		StarStruct star_data;
		star_data.type = star.typeString();
		star_data.distance = (float)(star.uPosition - g.stars[0].uPosition).magnitude() / 2400000.0f;
		star_data.dyson_lumino = star.dysonLumino();
		for(PlanetData planet : star.planets)
		{
			PlanetStruct planet_data;
			planet_data.type = LDB.Select(planet.theme).DisplayName;
			planet_data.singularity = planet.GetPlanetSingularityVector();
			planet_data.is_in_dsp = star.dysonRadius > planet.maxorbitRadius * 0.6770833f;
			if(planet.waterItemId == 1000)
				planet_data.liquid = 1;//水
			else if(planet.waterItemId == 1116)
				planet_data.liquid = 2;//硫酸
			else
				planet_data.liquid = 0;//无
			planet_data.is_gas = planet.gasItems.size();
			star_data.planets.push_back(planet_data);
			galaxy_data.planet_type_nums[planet_id_to_type[planet.theme - 1]] += 1;
		}
		galaxy_data.stars.push_back(star_data);
		galaxy_data.star_type_nums[star.typeId()] += 1;
	}

	if(check_galaxy(galaxy_data,galaxy_condition_simple))
	{
		if(check_no_veins)
			return true;
	}
	else
	{
		return false;
	}

	for(int star_index=0;star_index<g.starCount;star_index++)
	{
		StarData& star = g.stars[star_index];
		StarStruct& star_data = galaxy_data.stars[star_index];
		for(int planet_index=0;planet_index<star.planetCount;planet_index++)
		{
			PlanetData& planet = star.planets[planet_index];
			PlanetStruct& planet_data = star_data.planets[planet_index];
			g.MyGenerateVeins(star,planet,planet_data.veins);
			for(int i=0;i<14;i++)
				star_data.veins[i] += planet_data.veins[i];
		}
		for(int i=0;i<14;i++)
			galaxy_data.veins[i] += star_data.veins[i];
	}
	return check_galaxy(galaxy_data,galaxy_condition);
}
