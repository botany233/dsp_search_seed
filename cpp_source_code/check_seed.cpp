#include "check_seed.hpp"
#include <stdexcept>
#include "astro_class.hpp"
#include "src/max_flow.hpp"

namespace {
// 同一候选只计数一次；数量为零时不触发生成。
template<class Range,class Check>
static bool check_count(Range&& candidates,int need,Check check) {
	int left = static_cast<int>(candidates.size());
	for(auto& candidate : candidates) {
		if(need <= 0)
			return true;
		if(left < need)
			return false;
		left--;
		if(check(candidate))
			need--;
	}
	return need <= 0;
}

template<class Condition,class Points,class Amounts>
static uint16_t missing_veins(const Condition& condition,const Points& points,const Amounts& amounts) {
	uint16_t mask = 0;
	for(int i=0;i<14;++i)
		if(condition.veins_point[i] > points[i]
			|| (condition.need_veins_amount && condition.veins_amount[i] > amounts[i]))
			mask |= uint16_t(1 << i);
	return mask;
}

static bool check_star_level_1(StarClassSimple& star,const StarCondition& sc,const ConditionManager& manager) {
	if(!(sc.type & star.type_mask) || sc.dyson_lumino > star.luminosity)
		return false;
	// 默认距离不限制支持范围内的星系，出生星距离无需生成位置。
	if(sc.distance < 999.0f && sc.distance > 0.0f && star.index != 0)
		star.galaxy->ensure_star_position();
	if(sc.distance < star.distance)
		return false;
	return true;
}

static bool check_planet_level_2(PlanetClassSimple& planet,const PlanetCondition& condition,const ConditionManager& manager) {
	if((condition.singularity & planet.singularity) != condition.singularity)
		return false;
	if(condition.dsp_level || condition.type != 0xFFFFFFFF || condition.need_veins) {
		planet.ensure_theme();
		if(condition.dsp_level > planet.dsp_level)
			return false;
		if(!(condition.type & planet.type_mask))
			return false;
		if((planet.vein_mask & condition.need_veins) != condition.need_veins)
			return false;
	}
	// 二级只判断主题与结构，矿点、矿量条件留到后续级别。
	for(std::size_t i=0;i<condition.moons.count;++i) {
		const PlanetCondition& moon = manager.planets[manager.references[condition.moons.begin+i].index];
		if(!check_count(planet.moons,moon.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_2(p,moon,manager); }))
			return false;
	}
	return true;
}

static bool check_star_level_2(StarClassSimple& star,const StarCondition& sc,const ConditionManager& manager) {
	if(!(sc.type & star.type_mask) || sc.dyson_lumino > star.luminosity)
		return false;
	// 默认距离不限制支持范围内的星系，出生星距离无需生成位置。
	if(sc.distance < 999.0f && sc.distance > 0.0f && star.index != 0)
		star.galaxy->ensure_star_position();
	if(sc.distance < star.distance)
		return false;
	if(sc.need_veins) {
		star.ensure_themes();
		star.set_vein_mask();
		if((star.vein_mask & sc.need_veins) != sc.need_veins)
			return false;
	}
	if(sc.planets.count != 0)
		star.ensure_planets();
	for(std::size_t i=0;i<sc.planets.count;++i) {
		const PlanetCondition& pc = manager.planets[manager.references[sc.planets.begin+i].index];
		if(!check_count(star.planets,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_2(p,pc,manager); }))
			return false;
	}
	return true;
}

static bool check_planet_level_3(PlanetClassSimple& planet,const PlanetCondition& condition,const ConditionManager& manager) {
	if((condition.singularity & planet.singularity) != condition.singularity)
		return false;
	if(condition.dsp_level || condition.type != 0xFFFFFFFF || condition.need_veins) {
		planet.ensure_theme();
		if(condition.dsp_level > planet.dsp_level)
			return false;
		if(!(condition.type & planet.type_mask))
			return false;
		if((planet.vein_mask & condition.need_veins) != condition.need_veins)
			return false;
	}
	// 先检查卫星，避免为已经不满足结构条件的母星生成地形。
	for(std::size_t i=0;i<condition.moons.count;++i) {
		const PlanetCondition& moon = manager.planets[manager.references[condition.moons.begin+i].index];
		if(!check_count(planet.moons,moon.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_3(p,moon,manager); }))
			return false;
	}
	if(!condition.need_veins)
		return true;
	planet.ensure_upper_veins(condition.need_veins_amount);
	if(missing_veins(condition,planet.veins_point,planet.veins_amount))
		return false;
	return true;
}

static bool check_star_level_3(StarClassSimple& star,const StarCondition& sc,const ConditionManager& manager) {
	if(!(sc.type & star.type_mask) || sc.dyson_lumino > star.luminosity)
		return false;
	// 默认距离不限制支持范围内的星系，出生星距离无需生成位置。
	if(sc.distance < 999.0f && sc.distance > 0.0f && star.index != 0)
		star.galaxy->ensure_star_position();
	if(sc.distance < star.distance)
		return false;
	if(sc.need_veins) {
		star.ensure_themes();
		star.set_vein_mask();
		if((star.vein_mask & sc.need_veins) != sc.need_veins)
			return false;
	}
	if(sc.planets.count != 0)
		star.ensure_planets();
	for(std::size_t i=0;i<sc.planets.count;++i) {
		const PlanetCondition& pc = manager.planets[manager.references[sc.planets.begin+i].index];
		if(!check_count(star.planets,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_3(p,pc,manager); }))
			return false;
	}
	if(!sc.need_veins)
		return true;
	// 恒星缓存由行星生成接口累加；已满足时无需继续生成其他行星。
	for(PlanetClassSimple& planet : star.planets) {
		uint16_t missing = missing_veins(sc,star.upper_veins_point,star.upper_veins_amount);
		if(!missing)
			break;
		if(planet.vein_mask & missing)
			planet.ensure_upper_veins(sc.need_veins_amount);
	}
	if(missing_veins(sc,star.upper_veins_point,star.upper_veins_amount))
		return false;
	return true;
}

static bool check_planet_level_4(PlanetClassSimple& planet,const PlanetCondition& condition,const ConditionManager& manager) {
	if((condition.singularity & planet.singularity) != condition.singularity)
		return false;
	if(condition.dsp_level || condition.type != 0xFFFFFFFF || condition.need_veins) {
		planet.ensure_theme();
		if(condition.dsp_level > planet.dsp_level)
			return false;
		if(!(condition.type & planet.type_mask))
			return false;
		if((planet.vein_mask & condition.need_veins) != condition.need_veins)
			return false;
	}
	// 先检查卫星，避免为已经不满足结构条件的母星生成地形。
	for(std::size_t i=0;i<condition.moons.count;++i) {
		const PlanetCondition& moon = manager.planets[manager.references[condition.moons.begin+i].index];
		if(!check_count(planet.moons,moon.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_4(p,moon,manager); }))
			return false;
	}
	if(!condition.need_veins)
		return true;
	planet.ensure_upper_veins(condition.need_veins_amount);
	if(missing_veins(condition,planet.veins_point,planet.veins_amount))
		return false;
	planet.ensure_real_veins();
	if(missing_veins(condition,planet.veins_point,planet.veins_amount))
		return false;
	return true;
}

static bool check_star_level_4(StarClassSimple& star,const StarCondition& sc,const ConditionManager& manager) {
	if(!(sc.type & star.type_mask) || sc.dyson_lumino > star.luminosity)
		return false;
	// 默认距离不限制支持范围内的星系，出生星距离无需生成位置。
	if(sc.distance < 999.0f && sc.distance > 0.0f && star.index != 0)
		star.galaxy->ensure_star_position();
	if(sc.distance < star.distance)
		return false;
	if(sc.need_veins) {
		star.ensure_themes();
		star.set_vein_mask();
		if((star.vein_mask & sc.need_veins) != sc.need_veins)
			return false;
	}
	if(sc.planets.count != 0)
		star.ensure_planets();
	for(std::size_t i=0;i<sc.planets.count;++i) {
		const PlanetCondition& pc = manager.planets[manager.references[sc.planets.begin+i].index];
		if(!check_count(star.planets,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_4(p,pc,manager); }))
			return false;
	}
	if(!sc.need_veins)
		return true;
	// 恒星缓存由行星生成接口累加；已满足时无需继续生成其他行星。
	for(PlanetClassSimple& planet : star.planets) {
		if(planet.vein_mask & sc.need_veins)
			planet.ensure_upper_veins(sc.need_veins_amount);
	}
	if(missing_veins(sc,star.upper_veins_point,star.upper_veins_amount))
		return false;
	// 此前只补齐了足够满足条件的上界，不能用未完成的汇总上界提前否决真实资源。
	for(PlanetClassSimple& planet : star.planets) {
		uint16_t missing = missing_veins(sc,star.real_veins_point,star.real_veins_amount);
		if(!missing)
			return true;
		if(planet.vein_mask & missing)
			planet.ensure_real_veins();
		if(missing_veins(sc,star.upper_veins_point,star.upper_veins_amount))
			return false;
	}
	return !missing_veins(sc,star.real_veins_point,star.real_veins_amount);
}

// 行星槽位预先分配，但字段只能在所属恒星完成初始化后访问。
template<class Check>
static bool check_planet_count(GalaxyClassSimple& galaxy,int need,Check check) {
	int left = galaxy.planetCount;
	for(StarClassSimple& star : galaxy.stars) {
		if(need <= 0)
			return true;
		if(left < need)
			return false;
		star.ensure_planets();
		for(PlanetClassSimple& planet : star.planets) {
			if(need <= 0)
				return true;
			if(left < need)
				return false;
			left--;
			if(check(planet))
				need--;
		}
	}
	return need <= 0;
}

static int endpoint_capacity(const ConditionManager& manager,ConditionRef ref) {
	if(ref.type == ConditionType::Star)
		return manager.stars[ref.index].satisfy_num;
	if(ref.type == ConditionType::Planet)
		return manager.planets[ref.index].satisfy_num;
	throw std::invalid_argument("绑定端点必须为恒星或行星条件");
}

template<class CheckPlanet,class CheckStar>
static std::vector<VectorLF3> bond_positions(GalaxyClassSimple& galaxy,
	ConditionRef condition,CheckPlanet check_planet,CheckStar check_star,const ConditionManager& manager) {
	std::vector<VectorLF3> result;
	if(condition.type == ConditionType::Planet) {
		const PlanetCondition& pc = manager.planets[condition.index];
		for(StarClassSimple& star : galaxy.stars) {
			star.ensure_planets();
			for(PlanetClassSimple& planet : star.planets)
				if(check_planet(planet,pc,manager)) result.push_back(star.position);
		}
	} else {
		const StarCondition& sc = manager.stars[condition.index];
		for(StarClassSimple& star : galaxy.stars)
			if(check_star(star,sc,manager)) result.push_back(star.position);
	}
	return result;
}

template<class CheckPlanet,class CheckStar>
static bool check_bond(GalaxyClassSimple& galaxy,const BondCondition& condition,CheckPlanet check_planet,CheckStar check_star,const ConditionManager& manager) {
	if(condition.satisfy_num <= 0)
		return true;
	const int limit1 = endpoint_capacity(manager,condition.con1);
	const int limit2 = endpoint_capacity(manager,condition.con2);
	if(limit1 <= 0 || limit2 <= 0)
		return false;
	galaxy.ensure_star_position();
	const std::vector<VectorLF3> pos1 = bond_positions(galaxy,condition.con1,check_planet,check_star,manager);
	if(pos1.size()*limit1 < condition.satisfy_num)
		return false;
	const std::vector<VectorLF3> pos2 = bond_positions(galaxy,condition.con2,check_planet,check_star,manager);
	if(pos2.size()*limit2 < condition.satisfy_num)
		return false;
	const int n1 = static_cast<int>(pos1.size());
	const int n2 = static_cast<int>(pos2.size());
	const int sink = n1+n2+1;
	MaxFlowGraph graph(sink+1);
	for(int i=0;i<n1;++i) graph.add_edge(0,i+1,limit1);
	for(int j=0;j<n2;++j) graph.add_edge(n1+j+1,sink,limit2);
	for(int i=0;i<n1;++i)
		for(int j=0;j<n2;++j)
			if((pos1[i]-pos2[j]).magnitude() <= condition.distance)
				graph.add_edge(i+1,n1+j+1);
	return graph.flow(0,sink,condition.satisfy_num);
}

static uint16_t galaxy_missing_upper(const GalaxyClassSimple& galaxy,const GalaxyMineralCondition& condition) {
	std::array<uint64_t,14> points{};
	std::array<uint64_t,14> amounts{};
	for(const StarClassSimple& star : galaxy.stars)
		for(int i=0;i<14;i++) {
			points[i] += star.upper_veins_point[i];
			amounts[i] += star.upper_veins_amount[i];
		}
	return missing_veins(condition,points,amounts);
}

static uint16_t galaxy_missing_real(const GalaxyClassSimple& galaxy,const GalaxyMineralCondition& condition) {
	std::array<uint64_t,14> points{};
	std::array<uint64_t,14> amounts{};
	for(const StarClassSimple& star : galaxy.stars)
		for(int i=0;i<14;i++) {
			points[i] += star.real_veins_point[i];
			amounts[i] += star.real_veins_amount[i];
		}
	return missing_veins(condition,points,amounts);
}

static bool check_galaxy_upper_vein(GalaxyClassSimple& galaxy,const GalaxyMineralCondition& condition) {
	uint16_t missing = galaxy_missing_upper(galaxy,condition);
	if(!missing)
		return true;
	for(StarClassSimple& star : galaxy.stars) {
		star.ensure_planets();
		for(PlanetClassSimple& planet : star.planets) {
			planet.ensure_theme();
			if(planet.vein_mask & missing) {
				if(planet.ensure_upper_veins(condition.need_veins_amount)) {
					missing = galaxy_missing_upper(galaxy,condition);
					if(!missing)
						return true;
				}
			}
		}
	}
	return false;
}

static bool check_galaxy_real_vein(GalaxyClassSimple& galaxy,const GalaxyMineralCondition& condition) {
	uint16_t missing = galaxy_missing_real(galaxy,condition);
	if(!missing)
		return true;
	for(StarClassSimple& star : galaxy.stars) {
		star.ensure_planets();
		for(PlanetClassSimple& planet : star.planets) {
			planet.ensure_theme();
			if(!(planet.vein_mask & missing))
				continue;
			if(planet.ensure_upper_veins(condition.need_veins_amount)) {
				if(!(planet.vein_mask & missing)) //vein_mask用upper收紧后不再跟需求重叠
					continue;
				if(!check_galaxy_upper_vein(galaxy,condition))
					return false;
			}
			if(planet.ensure_real_veins()) {
				missing = galaxy_missing_real(galaxy,condition);
				if(!missing)
					return true;
			}
		}
	}
	return false;
}

using CheckStar = bool (*)(StarClassSimple&,const StarCondition&,const ConditionManager&);
using CheckPlanet = bool (*)(PlanetClassSimple&,const PlanetCondition&,const ConditionManager&);
using CheckMineral = bool (*)(GalaxyClassSimple&,const GalaxyMineralCondition&);

struct Stage {
	CheckStar star;
	CheckPlanet planet;
	CheckMineral mineral;
};

// 各级核心检查独立实现；公共调度只负责集合、引用、三态结果和逻辑聚合。
const Stage stages[] = {
	{check_star_level_1,nullptr,nullptr},
	{check_star_level_2,check_planet_level_2,nullptr},
	{check_star_level_3,check_planet_level_3,check_galaxy_upper_vein},
	{check_star_level_4,check_planet_level_4,check_galaxy_real_vein}
};

EnumResult check_core(GalaxyClassSimple& galaxy,const ConditionManager& manager,ConditionRef ref,int level,bool quick) {
	const Stage& stage = stages[level-1];
	bool passed = false;
	int complete_level = 1;
	switch(ref.type) {
	case ConditionType::GalaxyMineral: {
		const GalaxyMineralCondition& condition = manager.galaxy_minerals[ref.index];
		if(!condition.need_veins)
			return EnumResult::True;
		if(!stage.mineral)
			return EnumResult::Unknown;
		passed = stage.mineral(galaxy,condition);
		complete_level = 3;
		break;
	}
	case ConditionType::Star: {
		const StarCondition& condition = manager.stars[ref.index];
		if(!condition.satisfy_num)
			return EnumResult::True;
		passed = check_count(galaxy.stars,condition.satisfy_num,[&](StarClassSimple& star) {
			return stage.star(star,condition,manager);
		});
		complete_level = condition.level;
		break;
	}
	case ConditionType::Planet: {
		const PlanetCondition& condition = manager.planets[ref.index];
		if(!condition.satisfy_num)
			return EnumResult::True;
		if(!stage.planet)
			return EnumResult::Unknown;
		passed = check_planet_count(galaxy,condition.satisfy_num,[&](PlanetClassSimple& planet) {
			return stage.planet(planet,condition,manager);
		});
		complete_level = condition.level;
		break;
	}
	case ConditionType::Bond: {
		const BondCondition& condition = manager.bonds[ref.index];
		if(condition.satisfy_num <= 0)
			return EnumResult::True;
		if(!stage.planet)
			return EnumResult::Unknown;
		passed = check_bond(galaxy,condition,stage.planet,stage.star,manager);
		complete_level = condition.level;
		break;
	}
	default: throw std::invalid_argument("不支持的核心条件类型");
	}
	if(!passed)
		return EnumResult::False;
	// 上界通过不能证明真实资源满足，尤其不能被 NOT 当作完整真值。
	if(complete_level == 3 && !quick)
		complete_level = 4;
	return level >= complete_level ? EnumResult::True : EnumResult::Unknown;
}
}

bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick) {
	GalaxyClassSimple workspace;
	return check_seed(seed,conditions,quick,workspace);
}

bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick,GalaxyClassSimple& workspace) {
	return check_seed(seed,conditions,quick,workspace,0);
}

bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick,GalaxyClassSimple& workspace,std::size_t start_index) {
	workspace.init(seed);
	ResultManager results = conditions.create_result_manager();
	for(int level=1;level<=(quick ? 3 : 4);level++) {
		if(level == 2)
			workspace.ready_for_level2_check();
		const std::size_t count = conditions.core_conditions.size();
		for(std::size_t i=0;i<count;i++) {
			const ConditionRef& ref = conditions.core_conditions[(start_index+i) % count];
			const EnumResult root = conditions.evaluate_root(results);
			if(root != EnumResult::Unknown)
				return root == EnumResult::True;
			EnumResult& result = results.results[ref.result_index];
			if(result != EnumResult::Unknown)
				continue;
			result = check_core(workspace,conditions,ref,level,quick);
		}
		const EnumResult root = conditions.evaluate_root(results);
		if(root != EnumResult::Unknown)
			return root == EnumResult::True;
	}
	throw std::logic_error("最终检查后仍有未知条件，请检查条件级别与引用");
}
