#include "json.hpp"
#include "DSPGen.hpp"

#include "check_seed_util.hpp"
#include "data_struct.hpp"
#include <iostream>
using namespace nlohmann;
using namespace std;

const int planet_theme_to_type[25] = {
	0, 22, 22, 1, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	21, 17, 18, 19, 20
};

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
				int veins[14] = {0};
				g.MyGenerateVeins(star,planet,veins);
				for(int i = 0; i < 14; i++)
				{
					planet_data.veins[i] = veins[i];
					star_data.veins[i] += veins[i];
				}
			}
			star_data.planets.push_back(planet_data);
			galaxy_data.planet_type_nums[planet_theme_to_type[planet.theme - 1]] += 1;
		}
		for(int i = 0; i < 14; i++)
			galaxy_data.veins[i] += star_data.veins[i];
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

bool check_seed(int seed,int star_num,const json& galaxy_condition,const json& galaxy_condition_simple,bool check_no_veins)
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
			planet_data.is_in_dsp = star.dysonRadius > planet.maxorbitRadius * 0.6770833f;
			planet_data.is_on_dsp = (1.083333f-planet.get_ion_enhance())*planet.maxorbitRadius <= 1.6f * star.dysonRadius;
			if(planet.waterItemId == 1000)
				planet_data.liquid = 1;//水
			else if(planet.waterItemId == 1116)
				planet_data.liquid = 2;//硫酸
			else
				planet_data.liquid = 0;//无
			planet_data.is_gas = planet.gasItems.size();
			star_data.planets.push_back(planet_data);
			//galaxy_data.planet_type_nums[planet_id_to_type[planet.theme - 1]] += 1;
		}
		galaxy_data.stars.push_back(star_data);
		//galaxy_data.star_type_nums[star.typeId()] += 1;
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
		StarStructSimple& star_data = galaxy_data.stars[star_index];
		for(int planet_index=0;planet_index<star.planetCount;planet_index++)
		{
			PlanetData& planet = star.planets[planet_index];
			PlanetStructSimple& planet_data = star_data.planets[planet_index];
			g.MyGenerateVeins(star,planet,planet_data.veins);
			for(int i=0;i<14;i++)
				star_data.veins[i] += planet_data.veins[i];
		}
		for(int i=0;i<14;i++)
			galaxy_data.veins[i] += star_data.veins[i];
	}
	return check_galaxy(galaxy_data,galaxy_condition);
}
