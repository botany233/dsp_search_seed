#include "check_seed.hpp"
#include "astro_class.hpp"
#include "src/max_flow.hpp"

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

static bool check_star_level_1(StarClassSimple& star,const StarCondition& sc) {
	if(!(sc.type & star.type_mask) || sc.dyson_lumino > star.luminosity)
		return false;
	// 默认距离不限制支持范围内的星系，出生星距离无需生成位置。
	if(sc.distance < 999.0f && sc.distance > 0.0f && star.index != 0)
		star.galaxy->ensure_star_position();
	if(sc.distance < star.distance)
		return false;
	return true;
}

static bool check_planet_level_2(PlanetClassSimple& planet,const PlanetCondition& condition) {
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
	for(const PlanetCondition& moon : condition.moons)
		if(!check_count(planet.moons,moon.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_2(p,moon); }))
			return false;
	return true;
}

static bool check_star_level_2(StarClassSimple& star,const StarCondition& sc) {
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
	if(!sc.planets.empty())
		star.ensure_planets();
	for(const PlanetCondition& pc : sc.planets)
		if(!check_count(star.planets,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_2(p,pc); }))
			return false;
	return true;
}

static bool check_planet_level_3(PlanetClassSimple& planet,const PlanetCondition& condition) {
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
	for(const PlanetCondition& moon : condition.moons)
		if(!check_count(planet.moons,moon.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_3(p,moon); }))
			return false;
	if(!condition.need_veins)
		return true;
	planet.ensure_upper_veins(condition.need_veins_amount);
	if(missing_veins(condition,planet.veins_point,planet.veins_amount))
		return false;
	return true;
}

static bool check_star_level_3(StarClassSimple& star,const StarCondition& sc) {
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
	if(!sc.planets.empty())
		star.ensure_planets();
	for(const PlanetCondition& pc : sc.planets)
		if(!check_count(star.planets,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_3(p,pc); }))
			return false;
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

static bool check_planet_level_4(PlanetClassSimple& planet,const PlanetCondition& condition) {
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
	for(const PlanetCondition& moon : condition.moons)
		if(!check_count(planet.moons,moon.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_4(p,moon); }))
			return false;
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

static bool check_star_level_4(StarClassSimple& star,const StarCondition& sc) {
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
	if(!sc.planets.empty())
		star.ensure_planets();
	for(const PlanetCondition& pc : sc.planets)
		if(!check_count(star.planets,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_4(p,pc); }))
			return false;
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

template<class CheckPlanet,class CheckStar>
static std::vector<VectorLF3> bond_positions(GalaxyClassSimple& galaxy,
	const std::variant<PlanetCondition,StarCondition>& condition,CheckPlanet check_planet,CheckStar check_star) {
	std::vector<VectorLF3> result;
	if(const PlanetCondition* pc = std::get_if<PlanetCondition>(&condition)) {
		for(StarClassSimple& star : galaxy.stars) {
			star.ensure_planets();
			for(PlanetClassSimple& planet : star.planets)
				if(check_planet(planet,*pc)) result.push_back(star.position);
		}
	} else {
		const StarCondition& sc = std::get<StarCondition>(condition);
		for(StarClassSimple& star : galaxy.stars)
			if(check_star(star,sc)) result.push_back(star.position);
	}
	return result;
}

template<class CheckPlanet,class CheckStar>
static bool check_bond(GalaxyClassSimple& galaxy,const BondCondition& condition,CheckPlanet check_planet,CheckStar check_star) {
	if(condition.satisfy_num <= 0) return true;
	const int limit1 = std::visit([](const auto& c) { return c.satisfy_num; },condition.con1);
	const int limit2 = std::visit([](const auto& c) { return c.satisfy_num; },condition.con2);
	if(!limit1 || !limit2) return false;
	galaxy.ensure_star_position();
	const auto pos1 = bond_positions(galaxy,condition.con1,check_planet,check_star);
	if(pos1.size()*limit1 < condition.satisfy_num) return false;
	const auto pos2 = bond_positions(galaxy,condition.con2,check_planet,check_star);
	if(pos2.size()*limit2 < condition.satisfy_num) return false;
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

static uint16_t galaxy_missing_upper(const GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
	std::array<uint16_t,14> points{};
	std::array<uint64_t,14> amounts{};
	for(const StarClassSimple& star : galaxy.stars)
		for(int i=0;i<14;i++) {
			points[i] += star.upper_veins_point[i];
			amounts[i] += star.upper_veins_amount[i];
		}
	return missing_veins(condition,points,amounts);
}

static uint16_t galaxy_missing_real(const GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
	std::array<uint16_t,14> points{};
	std::array<uint64_t,14> amounts{};
	for(const StarClassSimple& star : galaxy.stars)
		for(int i=0;i<14;i++) {
			points[i] += star.real_veins_point[i];
			amounts[i] += star.real_veins_amount[i];
		}
	return missing_veins(condition,points,amounts);
}

static bool check_galaxy_upper_vein(GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
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

static bool check_galaxy_real_vein(GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
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

static bool check_galaxy_level_1(GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
	for(const StarCondition& sc : condition.stars)
		if(!check_count(galaxy.stars,sc.satisfy_num,[&](StarClassSimple& s) { return check_star_level_1(s,sc); }))
			return false;
	return true;
}

static bool check_galaxy_level_2(GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
	for(const StarCondition& sc : condition.stars)
		if(!check_count(galaxy.stars,sc.satisfy_num,[&](StarClassSimple& s) { return check_star_level_2(s,sc); }))
			return false;
	for(const PlanetCondition& pc : condition.planets)
		if(!check_planet_count(galaxy,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_2(p,pc); }))
			return false;
	for(const BondCondition& bond : condition.bonds)
		if(!check_bond(galaxy,bond,check_planet_level_2,check_star_level_2))
			return false;
	return true;
}

static bool check_galaxy_level_3(GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
	for(const StarCondition& sc : condition.stars)
		if(!check_count(galaxy.stars,sc.satisfy_num,[&](StarClassSimple& s) { return check_star_level_3(s,sc); }))
			return false;
	for(const PlanetCondition& pc : condition.planets)
		if(!check_planet_count(galaxy,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_3(p,pc); }))
			return false;
	if(condition.need_veins) {
		if(!check_galaxy_upper_vein(galaxy,condition))
			return false;
	}
	for(const BondCondition& bond : condition.bonds)
		if(!check_bond(galaxy,bond,check_planet_level_3,check_star_level_3))
			return false;
	return true;
}

static bool check_galaxy_level_4(GalaxyClassSimple& galaxy,const GalaxyCondition& condition) {
	for(const StarCondition& sc : condition.stars)
		if(!check_count(galaxy.stars,sc.satisfy_num,[&](StarClassSimple& s) { return check_star_level_4(s,sc); }))
			return false;
	for(const PlanetCondition& pc : condition.planets)
		if(!check_planet_count(galaxy,pc.satisfy_num,[&](PlanetClassSimple& p) { return check_planet_level_4(p,pc); }))
			return false;
	if(condition.need_veins) {
		if(!check_galaxy_real_vein(galaxy,condition))
			return false;
	}
	for(const BondCondition& bond : condition.bonds)
		if(!check_bond(galaxy,bond,check_planet_level_4,check_star_level_4))
			return false;
	return true;
}

bool check_seed(const SeedStruct& seed,const GalaxyCondition& galaxy_condition,int check_level) {
	GalaxyClassSimple galaxy;
	galaxy.init(seed);
	// 同一级的全部条件通过后才升级，避免前面的资源条件抢先生成地形。
	if(!check_galaxy_level_1(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	if(check_level <= 1)
		return galaxy_condition.valid_state;
	galaxy.ready_for_level2_check();
	if(!check_galaxy_level_2(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	if(check_level <= 2)
		return galaxy_condition.valid_state;
	if(!check_galaxy_level_3(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	if(check_level <= 3)
		return galaxy_condition.valid_state;
	if(!check_galaxy_level_4(galaxy,galaxy_condition))
		return !galaxy_condition.valid_state;
	return galaxy_condition.valid_state;
}
