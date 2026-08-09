// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2025.10
#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>

#include "Maths.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "DotNet35Random.hpp"
#include "quaternion.hpp"

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
    MultipleSatellites = 0x20,
	Satellite = 0x40
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
const uint16_t planet_veins_mask[] = {
	0x0473,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x013B,
	0x02BF,
	0x1477,
	0x0B3F,
	0x0ABF,
	0x199F,
	0x0B37,
	0x003F,
	0x1573,
	0x147F,
	0x1060,
	0x093F,
	0x1477,
	0x0B3F,
	0x09BF,
	0x0000,
	0x147F,
	0x0C2B,
	0x09BF,
	0x167F
};
const uint16_t star_veins_mask[] = {
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0B00, //白矮
	0x2000, //中子
	0x2000, //黑洞
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000
};
const float resource_rates[] = {
	0.1f,0.3f,0.5f,0.8f,1.0f,1.5f,2.0f,3.0f,5.0f,8.0f,100.0f
};

class Pose {
public:
	Vector3 position;
	Quaternion rotation;
	// 构造函数
	Pose() {
		position = Vector3();
		rotation = Quaternion();
	};

	Pose(Vector3 position,Quaternion rotation) {
		this->position = position;
		this->rotation = rotation;
	};
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
	float obliquity; //倾角
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
	uint8_t singularity;
	int theme;
	int algoId;
	int style;
	PlanetClass* orbitAroundPlanet = NULL;
	std::vector<int> gasItems;
	std::vector<float> gasSpeeds;
	std::string display_name;
	Quaternion runtimeOrbitRotation;
	Quaternion runtimeSystemRotation;
	Vector3 birthPoint;
	Vector3 birthResourcePoint0;
	Vector3 birthResourcePoint1;
	int veins_point[14];
	uint64_t veins_amount[14];
	int type_id;

	inline float realRadius() const {
		return radius * scale;
	}

	inline float get_ion_enhance() const {
		float real_radius = realRadius();
		float temp = real_radius + ionHeight * 0.6f;
		return sqrt(temp*temp-real_radius*real_radius)/temp;
	}

	std::vector<std::string> GetPlanetSingularityVector() const {
		std::vector<std::string> singularityVector;
		if((singularity & EPlanetSingularity::TidalLocked) != EPlanetSingularity::None)
			singularityVector.push_back("潮汐锁定永昼永夜");
		if((singularity & EPlanetSingularity::TidalLocked2) != EPlanetSingularity::None)
			singularityVector.push_back("轨道共振1:2");
		if((singularity & EPlanetSingularity::TidalLocked4) != EPlanetSingularity::None)
			singularityVector.push_back("轨道共振1:4");
		if((singularity & EPlanetSingularity::LaySide) != EPlanetSingularity::None)
			singularityVector.push_back("横躺自转");
		if((singularity & EPlanetSingularity::ClockwiseRotate) != EPlanetSingularity::None)
			singularityVector.push_back("反向自转");
		if((singularity & EPlanetSingularity::MultipleSatellites) != EPlanetSingularity::None)
			singularityVector.push_back("多卫星");
		if((singularity & EPlanetSingularity::Satellite) != EPlanetSingularity::None)
			singularityVector.push_back("卫星");
		return singularityVector;
	}
};

class StarClass {
public:
	static constexpr std::string_view type_names[15] = {"红巨星","黄巨星","蓝巨星","白巨星","白矮星","中子星","黑洞","A型恒星","B型恒星","F型恒星","G型恒星","K型恒星","M型恒星","O型恒星","未知恒星类型"};

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

	std::vector<PlanetClass> planets;

	float physicsRadius() const {
		return radius * 1200;
	}

	float dysonLumino() const {
		return Mathf.Round((float)Math.Pow(luminosity,0.33000001311302185) * 1000.0f) / 1000.0f;
	}

	std::string typeString() const {
		return std::string(type_names[typeId()]);
	}

	int typeId() const {
		if(type == EStarType::GiantStar)
		{
			if(spectr <= ESpectrType::K)
				return 0; //红巨星
			else if(spectr <= ESpectrType::F)
				return 1; //黄巨星
			else if((spectr != ESpectrType::A))
				return 2; //蓝巨星
			else
				return 3; //白巨星
		} else if(type == EStarType::WhiteDwarf)
			return 4; //白矮星
		else if(type == EStarType::NeutronStar)
			return 5; //中子星
		else if(type == EStarType::BlackHole)
			return 6; //黑洞
		else if(type == EStarType::MainSeqStar)
		{
			if(spectr == ESpectrType::A)
				return 7; //A型恒星
			else if(spectr == ESpectrType::B)
				return 8; //B型恒星
			else if(spectr == ESpectrType::F)
				return 9; //F型恒星
			else if(spectr == ESpectrType::G)
				return 10; //G型恒星
			else if(spectr == ESpectrType::K)
				return 11; //K型恒星
			else if(spectr == ESpectrType::M)
				return 12; //M型恒星
			else if(spectr == ESpectrType::O)
				return 13; //O型恒星
			else
				return 14; //未知恒星
		} else
			return 14; //未知恒星
	}
};
