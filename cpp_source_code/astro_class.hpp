// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2026.2
#pragma once
#include <iostream>
#include <cstdint>
#include <vector>

#include "defines.hpp"
#include "LDB.hpp"
#include "util.hpp"
#include "quaternion.hpp"
#include "Vector3.hpp"
#include "VectorLF3.hpp"
#include "DotNet35Random.hpp"
#include "PlanetRawData.hpp"

class StarClassSimple;
class GalaxyClassSimple;

class PlanetClassSimple
{
public:
	int seed;
	int id;
	int index;
	int orbitAround;
	int number;
	int orbitIndex;
	float maxorbitRadius;
	float radius = 200.0f;
	float scale = 1.0f;
	//float sunDistance;
	//float ionHeight;
	//float landPercent;
	//float waterHeight;
	uint8_t waterItemId;
	uint8_t type_id;
	uint8_t singularity;
	uint8_t dsp_level;
	//bool is_upper_veins = false;
	bool is_real_veins = false;

	uint16_t has_veins = 0;
	uint16_t veins_group[14]{0};
	uint16_t veins_point[14]{0};
	//bool levelized;
	EPlanetType type;
	int theme;
	int algoId;
	double mod_x;
	double mod_y;
	PlanetClassSimple* orbitAroundPlanet = nullptr;
	std::vector<PlanetClassSimple*> moons;
	PlanetRawData data = PlanetRawData();
	float orbitRadius = 1.0f;
	float rotationPhase;
	float orbitInclination;
	float orbitPhase;
	double orbitalPeriod = 3600.0;
	double rotationPeriod = 480.0;
	StarClassSimple* star = nullptr;
	Quaternion runtimeOrbitRotation;
	Quaternion runtimeSystemRotation;
	Vector3 birthPoint;
	Vector3 birthResourcePoint0;
	Vector3 birthResourcePoint1;

	uint8_t typeId() {
		return planet_theme_to_type[theme-1];
	}

	float realRadius() {
		return radius * scale;
	}

	float get_ion_enhance(const float ionHeight) {
		float real_radius = realRadius();
		float temp = real_radius + ionHeight * 0.6f;
		return sqrt(temp*temp-real_radius*real_radius)/temp;
	}

	Pose PredictPose(double time)
	{
		double num = time / orbitalPeriod + (double)orbitPhase / 360.0;
		int num2 = (int)(num + 0.1);
		num -= (double)num2;
		num *= Math.PI * 2.0;
		double num3 = time / rotationPeriod + (double)rotationPhase / 360.0;
		int num4 = (int)(num3 + 0.1);
		num3 = (num3 - (double)num4) * 360.0;
		Vector3 position = Maths::QRotate(runtimeOrbitRotation,Vector3((float)Math.Cos(num) * orbitRadius,0.0f,(float)Math.Sin(num) * orbitRadius));
		if(orbitAroundPlanet != nullptr)
		{
			Pose pose = orbitAroundPlanet->PredictPose(time);
			position.x += pose.position.x;
			position.y += pose.position.y;
			position.z += pose.position.z;
		}
		return Pose(position,runtimeSystemRotation * Quaternion::AngleAxis((float)num3,Vector3::down()));
	}
	
	void GenBirthPoints(int _birthSeed,const VectorLF3& star_uPosition)
	{
		DotNet35Random dotNet35Random = DotNet35Random(_birthSeed);
		Pose pose = PredictPose(85.0);
		Vector3 vector = Maths::QInvRotateLF(pose.rotation,star_uPosition - pose.position * 40000.0);
		vector.Normalize();
		Vector3 normalized = Vector3::Normalize(Vector3::Cross(vector,Vector3::up()));
		Vector3 normalized2 = Vector3::Normalize(Vector3::Cross(normalized,vector));
		int i = 0;
		int num;
		for(num = 256; i < num; i++)
		{
			float num2 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.5f;
			float num3 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.5f;
			Vector3 vector2 = vector + normalized * num2 + normalized2 * num3;
			vector2.Normalize();
			birthPoint = vector2 * (realRadius() + 0.2f + 1.45f);
			normalized = Vector3::Normalize(Vector3::Cross(vector2,Vector3::up()));
			normalized2 = Vector3::Normalize(Vector3::Cross(normalized,vector2));
			bool flag = false;
			for(int j = 0; j < 10; j++)
			{
				float x = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0);
				float y = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0);
				Vector2 vector3 = Vector2::Normalize(Vector2(x,y)) * 0.1f;
				Vector2 vector4 = -vector3;
				float num4 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.06f;
				float num5 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.06f;
				vector4.x += num4;
				vector4.y += num5;
				Vector3 normalized3 = Vector3::Normalize((vector2 + normalized * vector3.x + normalized2 * vector3.y));
				Vector3 normalized4 = Vector3::Normalize((vector2 + normalized * vector4.x + normalized2 * vector4.y));
				birthResourcePoint0 = Vector3::Normalize(normalized3);
				birthResourcePoint1 = Vector3::Normalize(normalized4);
				float num6 = realRadius() + 0.2f;
				if(data.QueryHeight(vector2) > num6 && data.QueryHeight(normalized3) > num6 && data.QueryHeight(normalized4) > num6)
				{
					Vector3 vpos = normalized3 + normalized * 0.03f;
					Vector3 vpos2 = normalized3 - normalized * 0.03f;
					Vector3 vpos3 = normalized3 + normalized2 * 0.03f;
					Vector3 vpos4 = normalized3 - normalized2 * 0.03f;
					Vector3 vpos5 = normalized4 + normalized * 0.03f;
					Vector3 vpos6 = normalized4 - normalized * 0.03f;
					Vector3 vpos7 = normalized4 + normalized2 * 0.03f;
					Vector3 vpos8 = normalized4 - normalized2 * 0.03f;
					if(data.QueryHeight(vpos) > num6 && data.QueryHeight(vpos2) > num6 && data.QueryHeight(vpos3) > num6 && data.QueryHeight(vpos4) > num6 && data.QueryHeight(vpos5) > num6 && data.QueryHeight(vpos6) > num6 && data.QueryHeight(vpos7) > num6 && data.QueryHeight(vpos8) > num6)
					{
						flag = true;
						break;
					}
				}
			}
			if(flag)
			{
				break;
			}
		}
		if(i >= num)
		{
			birthPoint = Vector3(0.0f,realRadius() + 5.0f,0.0f);
		}
	}

	void MyGenerateVeins();

	void generate_real_veins();
};

class StarClassSimple
{
public:
	int seed;
	int index;
	int id; //index+1
	//VectorLF3 position; //单位：光年
	VectorLF3 uPosition; //单位：米
	float mass = 1.0f;
	float age;
	EStarType type;
	ESpectrType spectr;
	float luminosity = 1.0f;
	float radius = 1.0f;
	float habitableRadius = 1.0f;
	float dysonRadius = 10.0f;
	float orbitScaler = 1.0f;
	int planetCount;
	float distance;

	uint8_t type_id;
	uint16_t has_veins = 0;
	uint16_t upper_veins_group[14]{0};
	uint16_t upper_veins_point[14]{0};
	uint16_t real_veins_group[14]{0};
	uint16_t real_veins_point[14]{0};
	//float resourceCoef = 1.0f;
	std::vector<PlanetClassSimple> planets;
	GalaxyClassSimple* galaxy = nullptr;

	uint8_t typeId()
	{
		if(type == EStarType::GiantStar)
		{
			if(spectr <= ESpectrType::K)
				return 1; //红巨星
			else if(spectr <= ESpectrType::F)
				return 2; //黄巨星
			else if((spectr != ESpectrType::A))
				return 3; //蓝巨星
			else
				return 4; //白巨星
		} else if(type == EStarType::WhiteDwarf)
			return 5; //白矮星
		else if(type == EStarType::NeutronStar)
			return 6; //中子星
		else if(type == EStarType::BlackHole)
			return 7; //黑洞
		else if(type == EStarType::MainSeqStar)
		{
			if(spectr == ESpectrType::A)
				return 8; //A型恒星
			else if(spectr == ESpectrType::B)
				return 9; //B型恒星
			else if(spectr == ESpectrType::F)
				return 10; //F型恒星
			else if(spectr == ESpectrType::G)
				return 11; //G型恒星
			else if(spectr == ESpectrType::K)
				return 12; //K型恒星
			else if(spectr == ESpectrType::M)
				return 13; //M型恒星
			else if(spectr == ESpectrType::O)
				return 14; //O型恒星
			else
				return 15;
		} else
			return 16;
	}

	float physicsRadius() {
		return radius * 1200;
	}
};

class GalaxyClassSimple
{
protected:
	void SetPlanetTheme(StarClassSimple& star,PlanetClassSimple& planet,double rand1,double rand2,double rand3,double rand4,int theme_seed,float planet_temperatureBias)
	{
		std::vector <int> tmp_theme;
		std::vector<int>& themeIds = LDB.themeIds;
		int length1 = (int)themeIds.size();
		for(int index1 = 0; index1 < length1; ++index1)
		{
			ThemeProto& themeProto = LDB.Select(themeIds[index1]);
			bool flag1 = false;
			if(star.index == 0 && planet.type == EPlanetType::Ocean)
			{
				if(themeProto.Distribute == EThemeDistribute::Birth)
					flag1 = true;
			} else
			{
				bool flag2 = (double)themeProto.Temperature * (double)planet_temperatureBias >= -0.100000001490116;
				if((double)Mathf.Abs(themeProto.Temperature) < 0.5 && themeProto.PlanetType == EPlanetType::Desert)
					flag2 = (double)Mathf.Abs(planet_temperatureBias) < (double)Mathf.Abs(themeProto.Temperature) + 0.100000001490116;
				if((themeProto.PlanetType == planet.type) && flag2)
				{
					if(star.index == 0)
					{
						if(themeProto.Distribute == EThemeDistribute::Default)
							flag1 = true;
					} else if(themeProto.Distribute == EThemeDistribute::Default || themeProto.Distribute == EThemeDistribute::Interstellar)
						flag1 = true;
				}
			}
			if(flag1)
			{
				for(int index2 = 0; index2 < planet.index; ++index2)
				{
					if(star.planets[index2].theme == themeProto.ID)
					{
						flag1 = false;
						break;
					}
				}
			}
			if(flag1)
				tmp_theme.push_back(themeProto.ID);
		}
		if(tmp_theme.size() == 0)
		{
			for(int index3 = 0; index3 < length1; ++index3)
			{
				ThemeProto& themeProto = LDB.Select(themeIds[index3]);
				bool flag = false;
				if(themeProto.PlanetType == EPlanetType::Desert)
					flag = true;
				if(flag)
				{
					for(int index4 = 0; index4 < planet.index; ++index4)
					{
						if(star.planets[index4].theme == themeProto.ID)
						{
							flag = false;
							break;
						}
					}
				}
				if(flag)
					tmp_theme.push_back(themeProto.ID);
			}
		}
		if(tmp_theme.size() == 0)
		{
			for(int index = 0; index < length1; ++index)
			{
				ThemeProto& themeProto = LDB.Select(themeIds[index]);
				if(themeProto.PlanetType == EPlanetType::Desert)
					tmp_theme.push_back(themeProto.ID);
			}
		}
		planet.theme = tmp_theme[(int)(rand1 * (double)tmp_theme.size()) % tmp_theme.size()];
		ThemeProto& themeProto1 = LDB.Select(planet.theme);
		planet.algoId = themeProto1.Algos[(int)(rand2 * (double)themeProto1.Algos.size()) % themeProto1.Algos.size()];
		planet.mod_x = (double)themeProto1.ModX.x + rand3 * ((double)themeProto1.ModX.y - (double)themeProto1.ModX.x);
		planet.mod_y = (double)themeProto1.ModY.x + rand4 * ((double)themeProto1.ModY.y - (double)themeProto1.ModY.x);

		//planet.display_name = themeProto1.DisplayName;
		//planet.style = theme_seed % 60;
		planet.type = themeProto1.PlanetType;
		//planet.ionHeight = themeProto1.IonHeight;
		//planet.windStrength = themeProto1.Wind;
		//planet.waterHeight = themeProto1.WaterHeight;
		planet.waterItemId = themeProto1.WaterItemId;
		//planet.levelized = themeProto1.UseHeightForBuild;
		if(themeProto1.Distribute == EThemeDistribute::Birth){
			birthPlanetId = planet.id;
		}
		if(star.dysonRadius > planet.maxorbitRadius * 52083.333f)
			planet.dsp_level = 2;
		else if((1.041667f-planet.get_ion_enhance(themeProto1.IonHeight))*planet.maxorbitRadius <= 0.00002f * star.dysonRadius)
			planet.dsp_level = 1;
		else
			planet.dsp_level = 0;
		planet.type_id = planet.typeId();
	}

	PlanetClassSimple CreatePlanet(StarClassSimple& star,int index,int orbitAround,int orbitIndex,int number,bool gasGiant,int info_seed,int gen_seed)
	{
		PlanetClassSimple& planet = star.planets[index];
		DotNet35Random dotNet35Random(info_seed);
		planet.index = index;
		planet.seed = gen_seed;
		planet.orbitAround = orbitAround;
		planet.orbitIndex = orbitIndex;
		planet.number = number;
		planet.id = star.id * 100 + index + 1;
		planet.star = &star;

		int num1 = 0;
		for(int index1 = 0; index1 < star.index; ++index1)
			num1 += stars[index1].planetCount;
		int num2 = num1 + index;
		if(orbitAround > 0)
		{
			planet.singularity |= EPlanetSingularity::Satellite;
			for(int index2 = 0; index2 < star.planetCount; ++index2)
			{
				if(orbitAround == star.planets[index2].number && star.planets[index2].orbitAround == 0)
				{
					planet.orbitAroundPlanet = &star.planets[index2];
					planet.orbitAroundPlanet->moons.push_back(&planet);
					if(orbitIndex > 1)
					{
						planet.orbitAroundPlanet->singularity |= EPlanetSingularity::MultipleSatellites;
						break;
					}
					break;
				}
			}
		}
		double num3 = dotNet35Random.NextDouble();
		double num4 = dotNet35Random.NextDouble();
		double num5 = dotNet35Random.NextDouble();
		double num6 = dotNet35Random.NextDouble();
		double num7 = dotNet35Random.NextDouble();
		double num8 = dotNet35Random.NextDouble();
		double num9 = dotNet35Random.NextDouble();
		double num10 = dotNet35Random.NextDouble();
		double num11 = dotNet35Random.NextDouble();
		double num12 = dotNet35Random.NextDouble();
		double num13 = dotNet35Random.NextDouble();
		double num14 = dotNet35Random.NextDouble();
		double rand1 = dotNet35Random.NextDouble();
		double num15 = dotNet35Random.NextDouble();
		double rand2 = dotNet35Random.NextDouble();
		double rand3 = dotNet35Random.NextDouble();
		double rand4 = dotNet35Random.NextDouble();
		int theme_seed = dotNet35Random.Next();
		float a = Mathf.Pow(1.2f,(float)(num3 * (num4 - 0.5) * 0.5));
		float f1;
		if(orbitAround == 0)
		{
			float b = orbitRadius[orbitIndex] * star.orbitScaler;
			float num16 = (float)(((double)a - 1.0) / (double)Mathf.Max(1.0f,b) + 1.0);
			f1 = b * num16;
		} else
			f1 = (float)(((1600.0 * (double)orbitIndex + 200.0) * (double)Mathf.Pow(star.orbitScaler,0.3f) * (double)Mathf.Lerp(a,1.0f,0.5f) + (double)planet.orbitAroundPlanet->realRadius()) / 40000.0);
		planet.orbitRadius = f1;
		planet.orbitInclination = (float)(num5 * 16.0 - 8.0);
		if(orbitAround > 0)
			planet.orbitInclination *= 2.2f;
		//planet.orbitLongitude = (float)(num6 * 360.0);
		if(star.type >= EStarType::NeutronStar)
		{
			if((double)planet.orbitInclination > 0.0)
				planet.orbitInclination += 3.0f;
			else
				planet.orbitInclination -= 3.0f;
		}
		planet.orbitalPeriod = planet.orbitAroundPlanet != NULL ? Math.Sqrt(39.4784176043574 * (double)f1 * (double)f1 * (double)f1 / 1.08308421068537E-08) : Math.Sqrt(39.4784176043574 * (double)f1 * (double)f1 * (double)f1 / (1.35385519905204E-06 * (double)star.mass));
		planet.orbitPhase = (float)(num7 * 360.0);
		float planet_obliquity;
		if(num15 < 0.0399999991059303)
		{
			planet_obliquity = (float)(num8 * (num9 - 0.5) * 20.0);
			if((double)planet_obliquity < 0.0)
				planet_obliquity -= 70.0f;
			else
				planet_obliquity += 70.0f;
			planet.singularity |= EPlanetSingularity::LaySide;
		} else if(num15 < 0.100000001490116)
		{
			planet_obliquity = (float)(num8 * (num9 - 0.5) * 80.0);
			if((double)planet_obliquity < 0.0)
				planet_obliquity -= 30.0f;
			else
				planet_obliquity += 30.0f;
		} else
			planet_obliquity = (float)(num8 * (num9 - 0.5) * 60.0);
		planet.rotationPeriod = (num10 * num11 * 1000.0 + 400.0) * (orbitAround == 0 ? (double)Mathf.Pow(f1,0.25f) : 1.0) * (gasGiant ? 0.200000002980232 : 1.0);
		if(!gasGiant)
		{
			if(star.type == EStarType::WhiteDwarf)
				planet.rotationPeriod *= 0.5;
			else if(star.type == EStarType::NeutronStar)
				planet.rotationPeriod *= 0.200000002980232;
			else if(star.type == EStarType::BlackHole)
				planet.rotationPeriod *= 0.150000005960464;
		}
		planet.rotationPhase = (float)(num12 * 360.0);
		float planet_sunDistance = orbitAround == 0 ? planet.orbitRadius : planet.orbitAroundPlanet->orbitRadius;
		planet.scale = 1.0f;
		planet.maxorbitRadius = orbitAround == 0 ? planet.orbitRadius : planet.orbitRadius + planet_sunDistance;

		double num17 = orbitAround == 0 ? planet.orbitalPeriod : planet.orbitAroundPlanet->orbitalPeriod;
		planet.rotationPeriod = 1.0 / (1.0 / num17 + 1.0 / planet.rotationPeriod);
		if(orbitAround == 0 && orbitIndex <= 4 && !gasGiant)
		{
			if(num15 > 0.959999978542328)
			{
				planet_obliquity *= 0.01f;
				planet.rotationPeriod = planet.orbitalPeriod;
				planet.singularity |= EPlanetSingularity::TidalLocked;
			} else if(num15 > 0.930000007152557)
			{
				planet_obliquity *= 0.1f;
				planet.rotationPeriod = planet.orbitalPeriod * 0.5;
				planet.singularity |= EPlanetSingularity::TidalLocked2;
			} else if(num15 > 0.899999976158142)
			{
				planet_obliquity *= 0.2f;
				planet.rotationPeriod = planet.orbitalPeriod * 0.25;
				planet.singularity |= EPlanetSingularity::TidalLocked4;
			}
		}
		if(num15 > 0.85 && num15 <= 0.9)
		{
			planet.rotationPeriod = -planet.rotationPeriod;
			planet.singularity |= EPlanetSingularity::ClockwiseRotate;
		}
		planet.runtimeOrbitRotation = Quaternion::AngleAxis((float)(num6 * 360.0),Vector3::up()) * Quaternion::AngleAxis(planet.orbitInclination,Vector3::forward());
		//planet.runtimeOrbitRotation = Quaternion::AngleAxis(planet.orbitLongitude,Vector3::up()) * Quaternion::AngleAxis(planet.orbitInclination,Vector3::forward());
		if(planet.orbitAroundPlanet != NULL)
		{
			planet.runtimeOrbitRotation = planet.orbitAroundPlanet->runtimeOrbitRotation * planet.runtimeOrbitRotation;
		}
		planet.runtimeSystemRotation = planet.runtimeOrbitRotation * Quaternion::AngleAxis(planet_obliquity,Vector3::forward());
		float habitableRadius = star.habitableRadius;
		float planet_temperatureBias = 0.0f;
		if(gasGiant)
		{
			planet.type = EPlanetType::Gas;
			planet.radius = 80.0f;
			planet.scale = 10.0f;
			//planet.habitableBias = 100.0f;
		} else
		{
			float num18 = Mathf.Ceil(starCount * 0.29f);
			if((double)num18 < 11.0)
				num18 = 11.0f;
			double num19 = (double)num18 - (double)habitableCount;
			float num20 = (float)(starCount - star.index);
			//float sunDistance = planet.sunDistance;
			float num21 = 1000.0f;
			float f2 = 1000.0f;
			if((double)habitableRadius > 0.0 && (double)planet_sunDistance > 0.0)
			{
				f2 = planet_sunDistance / habitableRadius;
				num21 = Mathf.Abs(Mathf.Log(f2));
			}
			float num22 = Mathf.Clamp(Mathf.Sqrt(habitableRadius),1.0f,2.0f) - 0.04f;
			double num23 = (double)num20;
			float num24 = Mathf.Clamp(Mathf.Lerp((float)(num19 / num23),0.35f,0.5f),0.08f,0.8f);
			float planet_habitableBias = num21 * num22;
			planet_temperatureBias = (float)(1.20000004768372 / ((double)f2 + 0.200000002980232) - 1.0);
			float num25 = Mathf.Pow(Mathf.Clamp01(planet_habitableBias / num24),num24 * 10.0f);
			if(num13 > (double)num25 && star.index > 0 || planet.orbitAround > 0 && planet.orbitIndex == 1 && star.index == 0)
			{
				planet.type = EPlanetType::Ocean;
				++habitableCount;
			} else if((double)f2 < 0.833333015441895)
			{
				float num26 = Mathf.Max(0.15f,(float)((double)f2 * 2.5 - 0.850000023841858));
				planet.type = num14 >= (double)num26 ? EPlanetType::Vocano : EPlanetType::Desert;
			} else if((double)f2 < 1.20000004768372)
			{
				planet.type = EPlanetType::Desert;
			} else
			{
				float num27 = (float)(0.899999976158142 / (double)f2 - 0.100000001490116);
				planet.type = num14 >= (double)num27 ? EPlanetType::Ice : EPlanetType::Desert;
			}
			planet.radius = 200.0f;
		}
		SetPlanetTheme(star,planet,rand1,rand2,rand3,rand4,theme_seed,planet_temperatureBias);
		//star.galaxy.astrosData[planet.id].uRadius = planet.realRadius();
		return planet;
	}
	
	float RandNormal(float averageValue,float standardDeviation,double r1,double r2) {
		return averageValue + standardDeviation * (float)(Math.Sqrt(-2.0 * Math.Log(1.0 - r1)) * Math.Sin(2.0 * Math.PI * r2));
	}

	void SetStarAge(StarClassSimple& star,double rn,double rt)
	{
		float num1 = (float)(rn * 0.1 + 0.95);
		float num2 = (float)(rt * 0.4 + 0.8);
		float num3 = (float)(rt * 9.0 + 1.0);
		if((double)star.age >= 1.0)
		{
			if((double)star.mass >= 18.0)
			{
				star.type = EStarType::BlackHole;
				star.spectr = ESpectrType::X;
				star.mass *= 2.5f * num2;
				star.radius *= 1.0f;
				star.luminosity *= 1.0f / 1000.0f * num1;
				star.habitableRadius = 0.0f;
			} else if((double)star.mass >= 7.0)
			{
				star.type = EStarType::NeutronStar;
				star.spectr = ESpectrType::X;
				star.mass *= 0.2f * num1;
				star.radius *= 0.15f;
				star.luminosity *= 0.1f * num1;
				star.habitableRadius = 0.0f;
				star.orbitScaler *= 1.5f * num1;
			} else
			{
				star.type = EStarType::WhiteDwarf;
				star.spectr = ESpectrType::X;
				star.mass *= 0.2f * num1;
				star.radius *= 0.2f;
				star.luminosity *= 0.04f * num2;
				star.habitableRadius *= 0.15f * num2;
			}
		} else
		{
			if((double)star.age < 0.959999978542328)
				return;
			float num4 = (float)(Math.Pow(5.0,Math.Abs(Math.Log10((double)star.mass) - 0.7)) * 5.0);
			if((double)num4 > 10.0)
				num4 = (float)(((double)Mathf.Log(num4 * 0.1f) + 1.0) * 10.0);
			float num5 = (float)(1.0 - (double)Mathf.Pow(star.age,30.0f) * 0.5);
			star.type = EStarType::GiantStar;
			star.mass = num5 * star.mass;
			star.radius = num4 * num2;
			star.luminosity = 1.6f * star.luminosity;
			star.habitableRadius = 9.0f * star.habitableRadius;
			star.orbitScaler = 3.3f * star.orbitScaler;
		}
	}

	void CreateStar(VectorLF3 pos,int id,int seed,EStarType needtype,ESpectrType needSpectr)
	{
		StarClassSimple& star = stars[id - 1];
		star.index = id - 1;
		float level = (float)star.index / (float)(starCount - 1);
		star.id = id;
		star.seed = seed;
		DotNet35Random dotNet35Random1(seed);
		int seed1 = dotNet35Random1.Next();
		int Seed = dotNet35Random1.Next();
		//star.position = pos;
		star.uPosition = pos * 2400000.0;
		float num1 = (float)pos.magnitude() / 32.0f;
		if((double)num1 > 1.0)
			num1 = Mathf.Log(Mathf.Log(Mathf.Log(Mathf.Log(Mathf.Log(num1) + 1.0f) + 1.0f) + 1.0f) + 1.0f) + 1.0f;
		//star.resourceCoef = Mathf.Pow(7.0f,num1) * 0.6f;
		DotNet35Random dotNet35Random2(Seed);
		double r1 = dotNet35Random2.NextDouble();
		double r2 = dotNet35Random2.NextDouble();
		double num2 = dotNet35Random2.NextDouble();
		double rn = dotNet35Random2.NextDouble();
		double rt = dotNet35Random2.NextDouble();
		double num3 = (dotNet35Random2.NextDouble() - 0.5) * 0.2;
		double num4 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
		double y = dotNet35Random2.NextDouble() * 0.4 - 0.2;
		double num5 = Math.Pow(2.0,y);
		float num6 = Mathf.Lerp(-0.98f,0.88f,level);
		float averageValue = (double)num6 >= 0.0 ? num6 + 0.65f : num6 - 0.65f;
		float standardDeviation = 0.33f;
		if(needtype == EStarType::GiantStar)
		{
			averageValue = y > -0.08 ? -1.5f : 1.6f;
			standardDeviation = 0.3f;
		}
		float num7 = RandNormal(averageValue,standardDeviation,r1,r2);
		switch(needSpectr)
		{
		case ESpectrType::M:
		num7 = -3.0f;
		break;
		case ESpectrType::O:
		num7 = 3.0f;
		break;
		}
		float p1 = (float)((double)Mathf.Clamp((double)num7 <= 0.0 ? num7 * 1.0f : num7 * 2.0f,-2.4f,4.65f) + num3 + 1.0);
		switch(needtype)
		{
		case EStarType::WhiteDwarf:
		star.mass = (float)(1.0 + r2 * 5.0);
		break;
		case EStarType::NeutronStar:
		star.mass = (float)(7.0 + r1 * 11.0);
		break;
		case EStarType::BlackHole:
		star.mass = (float)(18.0 + r1 * r2 * 30.0);
		break;
		default:
		star.mass = Mathf.Pow(2.0f,p1);
		break;
		}
		switch(needtype)
		{
		case EStarType::GiantStar:
		star.age = (float)(num2 * 0.0399999991059303 + 0.959999978542328);
		break;
		case EStarType::WhiteDwarf:
		case EStarType::NeutronStar:
		case EStarType::BlackHole:
		star.age = (float)(num2 * 0.400000005960464 + 1.0);
		break;
		default:
		star.age = (double)star.mass >= 0.5 ? ((double)star.mass >= 0.8 ? (float)(num2 * 0.699999988079071 + 0.200000002980232) : (float)(num2 * 0.400000005960464 + 0.100000001490116)) : (float)(num2 * 0.119999997317791 + 0.0199999995529652);
		break;
		}
		float num9 = (float)(1.0 - (double)Mathf.Pow(Mathf.Clamp01(star.age),20.0f) * 0.5) * star.mass;
		float star_temperature = (float)(Math.Pow((double)num9,0.56 + 0.14 / (Math.Log10((double)num9 + 4.0) / Math.Log10(5.0))) * 4450.0 + 1300.0);
		double num10 = Math.Log10(((double)star_temperature - 1300.0) / 4500.0) / Math.Log10(2.6) - 0.5;
		if(num10 < 0.0)
			num10 *= 4.0;
		if(num10 > 2.0)
			num10 = 2.0;
		else if(num10 < -4.0)
			num10 = -4.0;
		star.spectr = (ESpectrType)Mathf.RoundToInt((float)num10 + 4.0f);
		star.luminosity = Mathf.Pow(num9,0.7f);
		star.radius = (float)(Math.Pow((double)star.mass,0.4) * num5);
		float p2 = (float)num10 + 2.0f;
		star.habitableRadius = Mathf.Pow(1.7f,p2) + 0.25f * Mathf.Min(1.0f,star.orbitScaler);
		star.orbitScaler = Mathf.Pow(1.35f,p2);
		if((double)star.orbitScaler < 1.0)
			star.orbitScaler = Mathf.Lerp(star.orbitScaler,1.0f,0.6f);
		SetStarAge(star,rn,rt);
		star.dysonRadius = star.orbitScaler * 0.28f;
		if((double)star.dysonRadius * 40000.0 < (double)star.physicsRadius() * 1.5)
			star.dysonRadius = (float)((double)star.physicsRadius() * 1.5 / 40000.0);
		star.dysonRadius = round(star.dysonRadius * 800) * 100;
		star.luminosity = Mathf.Round((float)Math.Pow(star.luminosity,0.33000001311302185) * 1000.0f) / 1000.0f;
		star.type_id = star.typeId();
		star.galaxy = this;
		star.distance = (float)(star.uPosition - stars[0].uPosition).magnitude() / 2400000.0f;
	}

	StarClassSimple CreateBirthStar(int index,int seed)
	{
		StarClassSimple& birthStar = stars[index];
		birthStar.index = 0;
		birthStar.id = 1;
		birthStar.seed = seed;
		//birthStar.resourceCoef = 0.6f;
		DotNet35Random dotNet35Random1(seed);
		int seed1 = dotNet35Random1.Next();
		int Seed = dotNet35Random1.Next();
		//birthStar.position = VectorLF3::zero();
		birthStar.uPosition = VectorLF3::zero();
		DotNet35Random dotNet35Random2(Seed);
		double r1 = dotNet35Random2.NextDouble();
		double r2 = dotNet35Random2.NextDouble();
		double num1 = dotNet35Random2.NextDouble();
		double rn = dotNet35Random2.NextDouble();
		double rt = dotNet35Random2.NextDouble();
		double num2 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
		double num3 = Math.Pow(2.0,dotNet35Random2.NextDouble() * 0.4 - 0.2);
		float p1 = Mathf.Clamp(RandNormal(0.0f,0.08f,r1,r2),-0.2f,0.2f);
		birthStar.mass = Mathf.Pow(2.0f,p1);
		birthStar.age = (float)(num1 * 0.4 + 0.3);
		float num4 = (float)(1.0 - (double)Mathf.Pow(Mathf.Clamp01(birthStar.age),20.0f) * 0.5) * birthStar.mass;
		float star_temperature = (float)(Math.Pow((double)num4,0.56 + 0.14 / (Math.Log10((double)num4 + 4.0) / Math.Log10(5.0))) * 4450.0 + 1300.0);
		double num5 = Math.Log10(((double)star_temperature - 1300.0) / 4500.0) / Math.Log10(2.6) - 0.5;
		if(num5 < 0.0)
			num5 *= 4.0;
		if(num5 > 2.0)
			num5 = 2.0;
		else if(num5 < -4.0)
			num5 = -4.0;
		birthStar.spectr = (ESpectrType)Mathf.RoundToInt((float)num5 + 4.0f);
		birthStar.luminosity = Mathf.Pow(num4,0.7f);
		birthStar.radius = (float)(Math.Pow((double)birthStar.mass,0.4) * num3);
		float p2 = (float)num5 + 2.0f;
		birthStar.habitableRadius = Mathf.Pow(1.7f,p2) + 0.2f * Mathf.Min(1.0f,birthStar.orbitScaler);
		birthStar.orbitScaler = Mathf.Pow(1.35f,p2);
		if((double)birthStar.orbitScaler < 1.0)
			birthStar.orbitScaler = Mathf.Lerp(birthStar.orbitScaler,1.0f,0.6f);
		SetStarAge(birthStar,rn,rt);
		birthStar.dysonRadius = birthStar.orbitScaler * 0.28f;
		if((double)birthStar.dysonRadius * 40000.0 < (double)birthStar.physicsRadius() * 1.5)
			birthStar.dysonRadius = (float)((double)birthStar.physicsRadius() * 1.5 / 40000.0);
		birthStar.galaxy = this;
		return birthStar;
	}

	int GenerateTempPoses(std::vector<VectorLF3>& poses,int seed,int targetCount,int iterCount,double minDist,double minStepLen,double maxStepLen,double flatten)
	{

		std::vector<VectorLF3> tmp_drunk;
		std::vector<VectorLF3> tmp_poses;
		RandomPoses(tmp_poses,tmp_drunk,seed,targetCount * iterCount,minDist,minStepLen,maxStepLen,flatten);
		poses.resize(targetCount);
		for(int i = 0; i < targetCount; i++)
			poses[i] = tmp_poses[i * 4];

		return (int)poses.size();
	}

	bool CheckCollision(std::vector<VectorLF3>& pts,VectorLF3& pt,double min_dist)
	{
		double num1 = min_dist * min_dist;
		for(auto& pt1 : pts)
		{
			double num2 = pt.x - pt1.x;
			double num3 = pt.y - pt1.y;
			double num4 = pt.z - pt1.z;
			if(num2 * num2 + num3 * num3 + num4 * num4 < num1)
				return true;
		}
		return false;
	}

	void RandomPoses(std::vector<VectorLF3>& tmp_poses,std::vector<VectorLF3>& tmp_drunk,int seed,int maxCount,double minDist,double minStepLen,double maxStepLen,double flatten)
	{
		DotNet35Random dotNet35Random(seed);
		double num1 = dotNet35Random.NextDouble();
		tmp_poses.push_back(VectorLF3::zero());
		int num2 = 6;
		int num3 = 8;
		if(num2 < 1)
			num2 = 1;
		if(num3 < 1)
			num3 = 1;
		double num4 = (double)(num3 - num2);
		int num5 = (int)(num1 * num4 + (double)num2);
		for(int index = 0; index < num5; ++index)
		{
			int num6 = 0;
			while(num6++ < 256)
			{
				double num7 = dotNet35Random.NextDouble() * 2.0 - 1.0;
				double num8 = (dotNet35Random.NextDouble() * 2.0 - 1.0) * flatten;
				double num9 = dotNet35Random.NextDouble() * 2.0 - 1.0;
				double num10 = dotNet35Random.NextDouble();
				double d = num7 * num7 + num8 * num8 + num9 * num9;
				if(d <= 1.0 && d >= 1E-08)
				{
					double num11 = Math.Sqrt(d);
					double num12 = (num10 * (maxStepLen - minStepLen) + minDist) / num11;
					VectorLF3 pt(num7 * num12,num8 * num12,num9 * num12);
					if(!CheckCollision(tmp_poses,pt,minDist))
					{
						tmp_drunk.push_back(pt);
						tmp_poses.push_back(pt);
						if(tmp_poses.size() >= maxCount)
							return;
						break;
					}
				}
			}
		}
		int num13 = 0;
		while(num13++ < 256)
		{
			for(int index = 0; index < tmp_drunk.size(); ++index)
			{
				if(dotNet35Random.NextDouble() <= 0.7)
				{
					int num14 = 0;
					while(num14++ < 256)
					{
						double num15 = dotNet35Random.NextDouble() * 2.0 - 1.0;
						double num16 = (dotNet35Random.NextDouble() * 2.0 - 1.0) * flatten;
						double num17 = dotNet35Random.NextDouble() * 2.0 - 1.0;
						double num18 = dotNet35Random.NextDouble();
						double d = num15 * num15 + num16 * num16 + num17 * num17;
						if(d <= 1.0 && d >= 1E-08)
						{
							double num19 = Math.Sqrt(d);
							double num20 = (num18 * (maxStepLen - minStepLen) + minDist) / num19;
							VectorLF3 pt(tmp_drunk[index].x + num15 * num20,tmp_drunk[index].y + num16 * num20,tmp_drunk[index].z + num17 * num20);
							if(!CheckCollision(tmp_poses,pt,minDist))
							{
								tmp_drunk[index] = pt;
								tmp_poses.push_back(pt);
								if(tmp_poses.size() >= maxCount)
									return;
								break;
							}
						}
					}
				}
			}
		}
	}

	void CreateStarPlanets(StarClassSimple& star)
	{
		DotNet35Random dotNet35Random1(star.seed);
		dotNet35Random1.Next();
		dotNet35Random1.Next();
		dotNet35Random1.Next();
		DotNet35Random dotNet35Random2(dotNet35Random1.Next());
		double num1 = dotNet35Random2.NextDouble();
		double num2 = dotNet35Random2.NextDouble();
		double num3 = dotNet35Random2.NextDouble();
		double num4 = dotNet35Random2.NextDouble();
		double num5 = dotNet35Random2.NextDouble();
		double num6 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
		double num7 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
		if(star.type == EStarType::BlackHole)
		{
			star.planetCount = 1;
			star.planets.resize(star.planetCount);
			int info_seed = dotNet35Random2.Next();
			int gen_seed = dotNet35Random2.Next();
			star.planets[0] = CreatePlanet(star,0,0,3,1,false,info_seed,gen_seed);
		} else if(star.type == EStarType::NeutronStar)
		{
			star.planetCount = 1;
			star.planets.resize(star.planetCount);
			int info_seed = dotNet35Random2.Next();
			int gen_seed = dotNet35Random2.Next();
			star.planets[0] = CreatePlanet(star,0,0,3,1,false,info_seed,gen_seed);
		} else if(star.type == EStarType::WhiteDwarf)
		{
			if(num1 < 0.699999988079071)
			{
				star.planetCount = 1;
				star.planets.resize(star.planetCount);
				int info_seed = dotNet35Random2.Next();
				int gen_seed = dotNet35Random2.Next();
				star.planets[0] = CreatePlanet(star,0,0,3,1,false,info_seed,gen_seed);
			} else
			{
				star.planetCount = 2;
				star.planets.resize(star.planetCount);
				if(num2 < 0.300000011920929)
				{
					int info_seed1 = dotNet35Random2.Next();
					int gen_seed1 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,3,1,false,info_seed1,gen_seed1);
					int info_seed2 = dotNet35Random2.Next();
					int gen_seed2 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,0,4,2,false,info_seed2,gen_seed2);
				} else
				{
					int info_seed3 = dotNet35Random2.Next();
					int gen_seed3 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,4,1,true,info_seed3,gen_seed3);
					int info_seed4 = dotNet35Random2.Next();
					int gen_seed4 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,1,1,1,false,info_seed4,gen_seed4);
				}
			}
		} else if(star.type == EStarType::GiantStar)
		{
			if(num1 < 0.300000011920929)
			{
				star.planetCount = 1;
				star.planets.resize(star.planetCount);
				int info_seed = dotNet35Random2.Next();
				int gen_seed = dotNet35Random2.Next();
				star.planets[0] = CreatePlanet(star,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed,gen_seed);
			} else if(num1 < 0.800000011920929)
			{
				star.planetCount = 2;
				star.planets.resize(star.planetCount);
				if(num2 < 0.25)
				{
					int info_seed5 = dotNet35Random2.Next();
					int gen_seed5 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed5,gen_seed5);
					int info_seed6 = dotNet35Random2.Next();
					int gen_seed6 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,0,num3 > 0.5 ? 4 : 3,2,false,info_seed6,gen_seed6);
				} else
				{
					int info_seed7 = dotNet35Random2.Next();
					int gen_seed7 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,3,1,true,info_seed7,gen_seed7);
					int info_seed8 = dotNet35Random2.Next();
					int gen_seed8 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,1,1,1,false,info_seed8,gen_seed8);
				}
			} else
			{
				star.planetCount = 3;
				star.planets.resize(star.planetCount);
				if(num2 < 0.150000005960464)
				{
					int info_seed9 = dotNet35Random2.Next();
					int gen_seed9 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed9,gen_seed9);
					int info_seed10 = dotNet35Random2.Next();
					int gen_seed10 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,0,num3 > 0.5 ? 4 : 3,2,false,info_seed10,gen_seed10);
					int info_seed11 = dotNet35Random2.Next();
					int gen_seed11 = dotNet35Random2.Next();
					star.planets[2] = CreatePlanet(star,2,0,num3 > 0.5 ? 5 : 4,3,false,info_seed11,gen_seed11);
				} else if(num2 < 0.75)
				{
					int info_seed12 = dotNet35Random2.Next();
					int gen_seed12 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed12,gen_seed12);
					int info_seed13 = dotNet35Random2.Next();
					int gen_seed13 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,0,4,2,true,info_seed13,gen_seed13);
					int info_seed14 = dotNet35Random2.Next();
					int gen_seed14 = dotNet35Random2.Next();
					star.planets[2] = CreatePlanet(star,2,2,1,1,false,info_seed14,gen_seed14);
				} else
				{
					int info_seed15 = dotNet35Random2.Next();
					int gen_seed15 = dotNet35Random2.Next();
					star.planets[0] = CreatePlanet(star,0,0,num3 > 0.5 ? 4 : 3,1,true,info_seed15,gen_seed15);
					int info_seed16 = dotNet35Random2.Next();
					int gen_seed16 = dotNet35Random2.Next();
					star.planets[1] = CreatePlanet(star,1,1,1,1,false,info_seed16,gen_seed16);
					int info_seed17 = dotNet35Random2.Next();
					int gen_seed17 = dotNet35Random2.Next();
					star.planets[2] = CreatePlanet(star,2,1,2,2,false,info_seed17,gen_seed17);
				}
			}
		} else
		{
			double pGas[6];
			if(star.index == 0)
			{
				star.planetCount = 4;
				pGas[0] = 0.0;
				pGas[1] = 0.0;
				pGas[2] = 0.0;
			} else if(star.spectr == ESpectrType::M)
			{
				star.planetCount = num1 >= 0.1 ? (num1 >= 0.3 ? (num1 >= 0.8 ? 4 : 3) : 2) : 1;
				if(star.planetCount <= 3)
				{
					pGas[0] = 0.2;
					pGas[1] = 0.2;
				} else
				{
					pGas[0] = 0.0;
					pGas[1] = 0.2;
					pGas[2] = 0.3;
				}
			} else if(star.spectr == ESpectrType::K)
			{
				star.planetCount = num1 >= 0.1 ? (num1 >= 0.2 ? (num1 >= 0.7 ? (num1 >= 0.95 ? 5 : 4) : 3) : 2) : 1;
				if(star.planetCount <= 3)
				{
					pGas[0] = 0.18;
					pGas[1] = 0.18;
				} else
				{
					pGas[0] = 0.0;
					pGas[1] = 0.18;
					pGas[2] = 0.28;
					pGas[3] = 0.28;
				}
			} else if(star.spectr == ESpectrType::G)
			{
				star.planetCount = num1 >= 0.4 ? (num1 >= 0.9 ? 5 : 4) : 3;
				if(star.planetCount <= 3)
				{
					pGas[0] = 0.18;
					pGas[1] = 0.18;
				} else
				{
					pGas[0] = 0.0;
					pGas[1] = 0.2;
					pGas[2] = 0.3;
					pGas[3] = 0.3;
				}
			} else if(star.spectr == ESpectrType::F)
			{
				star.planetCount = num1 >= 0.35 ? (num1 >= 0.8 ? 5 : 4) : 3;
				if(star.planetCount <= 3)
				{
					pGas[0] = 0.2;
					pGas[1] = 0.2;
				} else
				{
					pGas[0] = 0.0;
					pGas[1] = 0.22;
					pGas[2] = 0.31;
					pGas[3] = 0.31;
				}
			} else if(star.spectr == ESpectrType::A)
			{
				star.planetCount = num1 >= 0.3 ? (num1 >= 0.75 ? 5 : 4) : 3;
				if(star.planetCount <= 3)
				{
					pGas[0] = 0.2;
					pGas[1] = 0.2;
				} else
				{
					pGas[0] = 0.1;
					pGas[1] = 0.28;
					pGas[2] = 0.3;
					pGas[3] = 0.35;
				}
			} else if(star.spectr == ESpectrType::B)
			{
				star.planetCount = num1 >= 0.3 ? (num1 >= 0.75 ? 6 : 5) : 4;
				if(star.planetCount <= 3)
				{
					pGas[0] = 0.2;
					pGas[1] = 0.2;
				} else
				{
					pGas[0] = 0.1;
					pGas[1] = 0.22;
					pGas[2] = 0.28;
					pGas[3] = 0.35;
					pGas[4] = 0.35;
				}
			} else if(star.spectr == ESpectrType::O)
			{
				star.planetCount = num1 >= 0.5 ? 6 : 5;
				pGas[0] = 0.1;
				pGas[1] = 0.2;
				pGas[2] = 0.25;
				pGas[3] = 0.3;
				pGas[4] = 0.32;
				pGas[5] = 0.35;
			} else
				star.planetCount = 1;
			star.planets.resize(star.planetCount);
			int num8 = 0;
			int num9 = 0;
			int orbitAround = 0;
			int num10 = 1;
			for(int index = 0; index < star.planetCount; ++index)
			{
				int info_seed = dotNet35Random2.Next();
				int gen_seed = dotNet35Random2.Next();
				double num11 = dotNet35Random2.NextDouble();
				double num12 = dotNet35Random2.NextDouble();
				bool gasGiant = false;
				if(orbitAround == 0)
				{
					++num8;
					if(index < star.planetCount - 1 && num11 < pGas[index])
					{
						gasGiant = true;
						if(num10 < 3)
							num10 = 3;
					}
					for(; star.index != 0 || num10 != 3; ++num10)
					{
						int num13 = star.planetCount - index;
						int num14 = 9 - num10;
						if(num14 > num13)
						{
							float a = (float)num13 / (float)num14;
							float num15 = num10 <= 3 ? Mathf.Lerp(a,1.0f,0.15f) + 0.01f : Mathf.Lerp(a,1.0f,0.45f) + 0.01f;
							if(dotNet35Random2.NextDouble() < (double)num15)
								goto label_62;
						} else
							goto label_62;
					}
					gasGiant = true;
				} else
				{
					++num9;
					gasGiant = false;
				}
				label_62:
				star.planets[index] = CreatePlanet(star,index,orbitAround,orbitAround == 0 ? num10 : num9,orbitAround == 0 ? num8 : num9,gasGiant,info_seed,gen_seed);
				++num10;
				if(gasGiant)
				{
					orbitAround = num8;
					num9 = 0;
				}
				if(num9 >= 1 && num12 < 0.8)
				{
					orbitAround = 0;
					num9 = 0;
				}
			}
		}
	}

public:
	int seed;
	int starCount;
	std::vector<StarClassSimple> stars;
	int habitableCount;
	int birthPlanetId;
	int veins_group[14]{0};
	int veins_point[14]{0};

	void CreateStars(int galaxySeed,int starCount)
	{
		DotNet35Random dotNet35Random(galaxySeed);
		std::vector<VectorLF3> tmp_poses;
		int tempPoses = GenerateTempPoses(tmp_poses,dotNet35Random.Next(),starCount,4,2.0,2.3,3.5,0.18);
		this->seed = galaxySeed;
		this->starCount = tempPoses;
		this->stars.resize(tempPoses);
		this->habitableCount = 0;

		float num1 = (float)dotNet35Random.NextDouble();
		float num2 = (float)dotNet35Random.NextDouble();
		float num3 = (float)dotNet35Random.NextDouble();
		float num4 = (float)dotNet35Random.NextDouble();

		int num5 = Mathf.CeilToInt((float)(0.00999999977648258 * (double)tempPoses + (double)num1 * 0.300000011920929));
		int num6 = Mathf.CeilToInt((float)(0.00999999977648258 * (double)tempPoses + (double)num2 * 0.300000011920929));
		int num7 = Mathf.CeilToInt((float)(0.0160000007599592 * (double)tempPoses + (double)num3 * 0.400000005960464));
		int num8 = Mathf.CeilToInt((float)(0.0130000002682209 * (double)tempPoses + (double)num4 * 1.39999997615814));
		int num9 = tempPoses - num5;
		int num10 = num9 - num6;
		int num11 = num10 - num7;
		int num12 = (num11 - 1) / num8;
		int num13 = num12 / 2;
		for(int index = 0; index < tempPoses; ++index)
		{
			int seed = dotNet35Random.Next();
			if(index == 0)
			{
				stars[index] = CreateBirthStar(index,seed);
			} else
			{
				ESpectrType needSpectr = ESpectrType::X;
				if(index == 3)
					needSpectr = ESpectrType::M;
				else if(index == num11 - 1)
					needSpectr = ESpectrType::O;
				EStarType needtype = EStarType::MainSeqStar;
				if(index % num12 == num13)
					needtype = EStarType::GiantStar;
				if(index >= num9)
					needtype = EStarType::BlackHole;
				else if(index >= num10)
					needtype = EStarType::NeutronStar;
				else if(index >= num11)
					needtype = EStarType::WhiteDwarf;
				CreateStar(tmp_poses[index],index + 1,seed,needtype,needSpectr);
			}
		}
	}

	void CreatePlanets()
	{
		for(StarClassSimple& star: stars)
			CreateStarPlanets(star);
	}

};
