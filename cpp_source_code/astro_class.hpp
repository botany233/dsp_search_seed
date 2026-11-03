// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2026.2
#pragma once
#include <array>
#include <algorithm>
#include "data_struct.hpp"
#include "defines.hpp"
#include <cstdint>
#include <vector>
#include <span>
#ifdef __AVX2__
#include <immintrin.h>
#endif

#include "LDB.hpp"
#include "src/util.hpp"
#include "src/quaternion.hpp"
#include "src/Vector3.hpp"
#include "src/VectorLF3.hpp"
#include "src/DotNet35Random.hpp"
#include "const_value.hpp"

#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4838)

enum EVeinsState : uint8_t {
	NoVeins = 0, // 无矿物
	UpperPoints = 1 << 0, // 矿点上限
	UpperAmounts = 1 << 1, // 矿量上限
	Real = 1 << 2 // 真实矿物
};

class StarClassSimple;
class GalaxyClassSimple;

// 行星直接持有基础信息和一组资源，通过状态位标记资源的生成情况。
class PlanetClassSimple
{
public:
	int seed;
	int info_seed;
	int id;
	int index;
	int orbitAround;
	int number;
	int orbitIndex;
	float maxorbitRadius;
	uint8_t waterItemId;
	uint8_t singularity;
	uint8_t dsp_level;
	uint8_t veins_state;
	uint32_t type_mask;

	EPlanetType type;
	int theme;
	float temperatureBias;
	int algoId;
	float orbitRadius;
	double mod_x;
	double mod_y;
	PlanetClassSimple* orbitAroundPlanet = nullptr;
	std::span<PlanetClassSimple> moons;
	StarClassSimple* star;

	uint16_t vein_mask;
	std::array<uint16_t,14> veins_point{};
	std::array<uint64_t,14> veins_amount{};
	std::array<float,14> veins_amount_percent;

	PlanetClassSimple() {}
	void init(StarClassSimple& parent,int index,int orbitAround,int orbitIndex,int number,bool gasGiant,int info_seed,int gen_seed);
	bool ensure_theme();
	bool ensure_upper_veins(bool amount);
	bool ensure_real_veins();
private:
	void generate_theme();
	void generate_upper_points();
	void generate_upper_amount();
};

// 恒星管理自身行星范围，通过 galaxy 保证跨恒星的生成顺序。
class StarClassSimple
{
public:
	int planet_num;
	bool planets_initialized;
	int seed;
	int index;
	int id;
	int planet_num_seed;
	float resourceCoef;
	VectorLF3 position; //单位：LY
	VectorLF3 uPosition; //单位：m
	float mass;
	float age;
	EStarType type;
	ESpectrType spectr;
	float luminosity;
	float radius;
	float habitableRadius;
	float dysonRadius;
	float orbitScaler;
	float distance;
	std::span<PlanetClassSimple> planets;
	int themed_planet_count;
	uint8_t type_id;

	uint16_t type_mask;
	GalaxyClassSimple* galaxy;
	uint16_t vein_mask;
	std::array<uint16_t,14> upper_veins_point;
	std::array<uint64_t,14> upper_veins_amount;
	std::array<uint16_t,14> real_veins_point;
	std::array<uint64_t,14> real_veins_amount;

	StarClassSimple() {}
	void init(GalaxyClassSimple& parent,int index,int seed,EStarType needtype,ESpectrType needSpectr);
	void ensure_planets(); //初始化star的所有planet
	void ensure_themes(); //设置star的所有planet主题
	void set_vein_mask();
private:
	void init_birth(int seed);
	void init_planets();
	void SetStarAge(double rn,double rt);
};

// 搜索用轻量星系生成器，按检查阶段生成所需数据。
class GalaxyClassSimple
{
public:
	int generated_planets;
	int seed;
	int starCount;
	int planetCount;
	float resource_multiplier;
	int habitableCount;
	int birthPlanetId;
	bool is_rare_resource;
	bool is_infinite_resource;
	bool is_gen_position;
	int generated_stars;
	std::vector<StarClassSimple> stars;
	std::vector<PlanetClassSimple> planets;

	GalaxyClassSimple() {}
	GalaxyClassSimple(const GalaxyClassSimple&) = delete;
	GalaxyClassSimple& operator=(const GalaxyClassSimple&) = delete;
	void init(const SeedStruct& input);
	void ensure_star_position();
	void ready_for_level2_check();
};
