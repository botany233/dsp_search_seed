// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2025.10
#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "util.hpp"
#include "PlanetRawData.hpp"

enum ESpectrType {
    M = 0,
    K = 1,
    G = 2,
    F = 3,
    A = 4,
    B = 5,
    O = 6,
    X = 7
};
enum EStarType {
    MainSeqStar = 0,
    GiantStar = 1,
    WhiteDwarf = 2,
    NeutronStar = 3,
    BlackHole = 4
};
enum EVeinType {
	None_vein = 0,
	Iron = 1,
	Copper = 2,
	Silicium = 3,
	Titanium = 4,
	Stone = 5,
	Coal = 6,
	Oil = 7,
	Fireice = 8,
	Diamond = 9,
	Fractal = 10,
	Crysrub = 11,
	Grat = 12,
	Bamboo = 13,
	Mag = 14,
	Max = 15
};
enum EPlanetSingularity
{
    None = 0,
    TidalLocked = 1,
    TidalLocked2 = 2,
    TidalLocked4 = 4,
    LaySide = 8,
    ClockwiseRotate = 0x10,
    MultipleSatellites = 0x20
};

enum class EPlanetType
{
    None = 0,
    Vocano = 1,
    Ocean = 2,
    Desert = 3,
    Ice = 4,
    Gas = 5
};

enum class EThemeDistribute
{
    Default = 0,
    Birth = 1,
    Interstellar = 2,
    Rare = 3
};

const float orbitRadius[17] = {
  0.0f,
  0.4f,
  0.7f,
  1.0f,
  1.4f,
  1.9f,
  2.5f,
  3.3f,
  4.3f,
  5.5f,
  6.9f,
  8.4f,
  10.0f,
  11.7f,
  13.5f,
  15.4f,
  17.5f
};
const int themeres[] = {
    0x00000000,
    0x000088e6,
    0x00060000,
    0x00060000,
    0x00020001,
    0x00020001,
    0x00000276,
    0x0000057e,
    0x0000a8ee,
    0x0000167e,
    0x0000957e,
    0x0000133e,
    0x0000166e,
    0x0001007e,
    0x0000aae6,
    0x0000a8fe,
    0x0000a0c0,
    0x0000127e,
    0x0000a8ee,
    0x0000167e,
    0x0000137e,
    0x00060000,
    0x000028fe,
    0x00001856,
    0x0000137e,
    0x00002cfe
};
const int planet_theme_to_type[25] = {
	1,23,23,2,2,3,4,5,6,7,
	8,9,10,11,12,13,14,15,16,17,
	22,18,19,20,21
};

class PlanetClass
{
public:
    int seed;
    int infoSeed;
    int id;
    int index;
    int orbitAround;
    int number;
    int orbitIndex;
    std::string name;
    std::string overrideName;
    float orbitRadius = 1.0f;
	float maxorbitRadius;
    float orbitInclination;
    float orbitLongitude;
    double orbitalPeriod = 3600.0;
    float orbitPhase;
    float obliquity;
    double rotationPeriod = 480.0;
    float rotationPhase;
    float radius = 200.0f;
    float scale = 1.0f;
    float sunDistance;
    float habitableBias;
    float temperatureBias;
    float ionHeight;
    float windStrength;
    float luminosity;
    float landPercent;
    double mod_x;
    double mod_y;
    float waterHeight;
    int waterItemId;
    bool levelized;
    int iceFlag;
    EPlanetType type;
    int singularity;
    int theme;
    int algoId;
    int style;
    PlanetClass* orbitAroundPlanet = NULL;
    std::vector<int> gasItems;
    std::vector<float> gasSpeeds;
	std::string display_name;
	PlanetRawData data;

    inline float realRadius() {
        return radius * scale;
    }

	int typeId() {
		return planet_theme_to_type[theme-1];
	}

	inline float get_ion_enhance() {
		float real_radius = realRadius();
		float temp = real_radius + ionHeight * 0.6f;
		return sqrt(temp*temp-real_radius*real_radius)/temp;
	}

    std::vector<std::string> GetPlanetSingularityVector()
    {
		std::vector<std::string> singularityVector;
        if (orbitAround > 0)
			singularityVector.push_back("卫星");
        if ((singularity & EPlanetSingularity::TidalLocked) != EPlanetSingularity::None)
			singularityVector.push_back("潮汐锁定永昼永夜");
        if ((singularity & EPlanetSingularity::TidalLocked2) != EPlanetSingularity::None)
			singularityVector.push_back("轨道共振1:2");
        if ((singularity & EPlanetSingularity::TidalLocked4) != EPlanetSingularity::None)
			singularityVector.push_back("轨道共振1:4");
        if ((singularity & EPlanetSingularity::LaySide) != EPlanetSingularity::None)
			singularityVector.push_back("横躺自转");
        if ((singularity & EPlanetSingularity::ClockwiseRotate) != EPlanetSingularity::None)
			singularityVector.push_back("反向自转");
        if ((singularity & EPlanetSingularity::MultipleSatellites) != EPlanetSingularity::None)
			singularityVector.push_back("多卫星");
        return singularityVector;
    }

	uint8_t GetPlanetSingularityMask()
	{
		uint8_t singularity_mask = 0;
		if(orbitAround > 0)
			singularity_mask |= 1;
		if((singularity & EPlanetSingularity::TidalLocked) != EPlanetSingularity::None)
			singularity_mask |= 1 << 2;
		if((singularity & EPlanetSingularity::TidalLocked2) != EPlanetSingularity::None)
			singularity_mask |= 1 << 3;
		if((singularity & EPlanetSingularity::TidalLocked4) != EPlanetSingularity::None)
			singularity_mask |= 1 << 4;
		if((singularity & EPlanetSingularity::LaySide) != EPlanetSingularity::None)
			singularity_mask |= 1 << 5;
		if((singularity & EPlanetSingularity::ClockwiseRotate) != EPlanetSingularity::None)
			singularity_mask |= 1 << 6;
		if((singularity & EPlanetSingularity::MultipleSatellites) != EPlanetSingularity::None)
			singularity_mask |= 1 << 1;
		return singularity_mask;
	}
};

class StarClass {
public:
    int seed;
    int index;
    int id;
    std::string name;
    std::string overrideName;
    VectorLF3 position;
    VectorLF3 uPosition;
    float mass = 1.0f;
    float lifetime = 50.0f;
    float age;
    EStarType type;
    float temperature = 8500.0f;
    ESpectrType spectr;
    float classFactor;
    float color;
    float luminosity = 1.0f;
    float radius = 1.0f;
    float acdiskRadius;
    float habitableRadius = 1.0f;
    float lightBalanceRadius = 1.0f;
    float dysonRadius = 10.0f;
    float orbitScaler = 1.0f;
    float asterBelt1OrbitIndex;
    float asterBelt2OrbitIndex;
    float asterBelt1Radius;
    float asterBelt2Radius;
    int planetCount;
    float level;
    float resourceCoef = 1.0f;

    std::vector< PlanetClass> planets;

    inline float physicsRadius() {
        return radius * 1200;
    }

    inline float dysonLumino() {
        return Mathf.Round((float)Math.Pow(luminosity, 0.33000001311302185) * 1000.0f) / 1000.0f;
    }


    inline std::string typeString()
    {
        std::string text = "";
        if (type == EStarType::GiantStar)
        {
            text = ((spectr <= ESpectrType::K) ? (text + "红巨星") : ((spectr <= ESpectrType::F) ? (text + "黄巨星") : ((spectr != ESpectrType::A) ? (text + "蓝巨星") : (text + "白巨星"))));
        }
        else if (type == EStarType::WhiteDwarf)
        {
            text += "白矮星";
        }
        else if (type == EStarType::NeutronStar)
        {
            text += "中子星";
        }
        else if (type == EStarType::BlackHole)
        {
            text += "黑洞";
        }
        else if (type == EStarType::MainSeqStar)
        {
            if(spectr == ESpectrType::A)
                text = text + "A型恒星";
            else if (spectr == ESpectrType::B)
                text = text + "B型恒星";
            else if (spectr == ESpectrType::F)
                text = text + "F型恒星";
            else if (spectr == ESpectrType::G)
                text = text + "G型恒星";
            else if (spectr == ESpectrType::K)
                text = text + "K型恒星";
            else if (spectr == ESpectrType::M)
                text = text + "M型恒星";
            else if (spectr == ESpectrType::O)
                text = text + "O型恒星";
            else
                text = text + "X型恒星";
        }
        return text;
    }

	inline int typeId()
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
};
