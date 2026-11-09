#include "condition_struct.hpp"

#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <pybind11/stl.h>

namespace {
constexpr std::size_t max_index = std::numeric_limits<std::uint16_t>::max();
constexpr std::uint32_t liquid_masks[] = {0xFFFFFFFF,0x00025c51,0x00000200};

template<class T>
void set_need_veins(T& condition) {
	for(std::size_t i = 0;i < condition.veins_point.size();++i) {
		if(condition.veins_point[i] > 0 || condition.veins_amount[i] > 0)
			condition.need_veins |= static_cast<std::uint16_t>(1u << i);
		if(condition.veins_amount[i] > 0)
			condition.need_veins_amount = true;
	}
}

template<class T>
ConditionRef append_condition(std::vector<T>& pool,ConditionType type,const T& condition) {
	if(pool.size() > max_index)
		throw std::overflow_error("条件池超出 uint16_t 索引范围");
	const auto index = static_cast<std::uint16_t>(pool.size());
	pool.push_back(condition);
	return {type,index};
}
}

ResultManager::ResultManager(std::size_t core_condition_count)
	: results(core_condition_count,EnumResult::Unknown) {}

ResultManager ConditionManager::create_result_manager() const {
	return ResultManager(core_conditions.size());
}

// 去重表只在构造期间使用，结果下标写回 root、逻辑叶子及核心列表。
static void collect_core_conditions(ConditionManager& manager,ConditionRef& ref, std::unordered_map<std::uint32_t,std::uint16_t>& indices) {
	if(ref.type == ConditionType::Logic) {
		const LogicCondition& node = manager.logic_nodes[ref.index];
		for(std::size_t i=0;i<node.children_count;++i)
			collect_core_conditions(manager,manager.references[node.children_begin+i],indices);
		return;
	}
	const std::uint32_t key = (static_cast<std::uint32_t>(ref.type)<<16) | ref.index;
	const std::unordered_map<std::uint32_t,std::uint16_t>::const_iterator existing = indices.find(key);
	if(existing != indices.end()) {
		ref.result_index = existing->second;
		return;
	}
	// 最大值保留为无结果标记，不能作为有效结果下标。
	if(manager.core_conditions.size() >= max_index)
		throw std::overflow_error("核心结果超出 uint16_t 有效索引范围");
	ref.result_index = static_cast<std::uint16_t>(manager.core_conditions.size());
	indices.emplace(key,ref.result_index);
	manager.core_conditions.push_back(ref);
}

EnumResult ConditionManager::evaluate_root(const ResultManager& result_manager) const {
	if(result_manager.results.size() != core_conditions.size())
		throw std::invalid_argument("结果数量与核心条件不匹配");
	const auto evaluate = [&](const auto& self,const ConditionRef ref) -> EnumResult {
		switch(ref.type) {
		case ConditionType::GalaxyMineral:
		case ConditionType::Star:
		case ConditionType::Planet:
		case ConditionType::Bond:
			if(ref.result_index >= result_manager.results.size())
				throw std::invalid_argument("核心引用没有有效结果索引");
			return result_manager.results[ref.result_index];
		case ConditionType::Logic: {
			const LogicCondition& node = logic_nodes[ref.index];
			if(node.children_count == 0)
				return EnumResult::True;
			if(node.satisfy_num == 0) {
				const EnumResult child = self(self,references[node.children_begin]);
				return child == EnumResult::Unknown ? EnumResult::Unknown :
					(child == EnumResult::True ? EnumResult::False : EnumResult::True);
			}
			if(node.satisfy_num > node.children_count)
				return EnumResult::False;
			std::size_t true_count = 0,possible_count = node.children_count;
			for(std::size_t i = 0;i < node.children_count;++i) {
				const EnumResult child = self(self,references[node.children_begin + i]);
				if(child == EnumResult::True) {
					true_count++;
					if(true_count >= node.satisfy_num)
						return EnumResult::True;
				}
				if(child == EnumResult::False) {
					possible_count--;
					if(possible_count < node.satisfy_num)
						return EnumResult::False;
				}
			}
			return EnumResult::Unknown;
		}
		default:
			throw std::invalid_argument("不支持的条件类型");
		}
	};
	return evaluate(evaluate,root);
}

ConditionManager::ConditionManager(const std::string& galaxy_condition_json)
	: ConditionManager(py::module_::import("json").attr("loads")(galaxy_condition_json).cast<py::dict>()) {}

ConditionManager::ConditionManager(const py::dict& galaxy_condition_dict) {
	root = handle_condition_dict(galaxy_condition_dict);
	std::unordered_map<std::uint32_t,std::uint16_t> indices;
	collect_core_conditions(*this,root,indices);
	for(const GalaxyMineralCondition& condition : galaxy_minerals)
		all_need_veins |= condition.need_veins;
	for(const StarCondition& condition : stars)
		all_need_veins |= condition.need_veins;
	for(const PlanetCondition& condition : planets)
		all_need_veins |= condition.need_veins;
}

ConditionRef ConditionManager::handle_condition_dict(const py::dict& condition_dict) {
	const auto type = static_cast<ConditionType>(condition_dict["type"].cast<std::uint8_t>());
	const auto value = condition_dict["value"].cast<py::dict>();
	switch(type) {
	case ConditionType::GalaxyMineral:
		return add_galaxy_mineral_condition(value);
	case ConditionType::Star:
		return add_star_condition(value);
	case ConditionType::Planet:
		return add_planet_condition(value);
	case ConditionType::Bond:
		return add_bond_condition(value);
	case ConditionType::Logic:
		return add_logic_condition(value);
	default:
		throw std::invalid_argument("不支持的条件类型");
	}
}

static RefRange add_references(ConditionManager& manager, const std::vector<ConditionRef>& children) {
	if(manager.references.size() + children.size() > max_index)
		throw std::overflow_error("references已满！");
	const RefRange range{static_cast<std::uint16_t>(manager.references.size()), static_cast<std::uint16_t>(children.size())};
	// 子树先解析完毕，再一次性追加当前列表，保证片段连续。
	manager.references.insert(manager.references.end(),children.begin(),children.end());
	return range;
}

ConditionRef ConditionManager::add_galaxy_mineral_condition(const py::dict& condition) {
	GalaxyMineralCondition new_condition;
	new_condition.veins_point = condition["veins_point"].cast<std::array<std::uint16_t,14>>();
	new_condition.veins_amount = condition["veins_amount"].cast<std::array<std::uint64_t,14>>();
	set_need_veins(new_condition);
	return add_galaxy_mineral_condition(new_condition);
}

ConditionRef ConditionManager::add_galaxy_mineral_condition(const GalaxyMineralCondition& condition) {
	return append_condition(galaxy_minerals,ConditionType::GalaxyMineral,condition);
}

static std::uint8_t get_ref_level(const ConditionManager& manager,const ConditionRef& ref) {
	switch(ref.type) {
	case ConditionType::GalaxyMineral:
		return 3;
	case ConditionType::Star:
		return manager.stars[ref.index].level;
	case ConditionType::Planet:
		return manager.planets[ref.index].level;
	case ConditionType::Bond:
		return manager.bonds[ref.index].level;
	default:
		throw std::invalid_argument("不支持的条件类型");
	}
}

ConditionRef ConditionManager::add_star_condition(const py::dict& condition) {
	StarCondition new_condition;
	new_condition.distance = condition["distance"].cast<float>();
	new_condition.dyson_lumino = condition["dyson_lumino"].cast<float>();
	new_condition.type = condition["type"].cast<std::uint32_t>();
	new_condition.satisfy_num = condition["satisfy_num"].cast<std::uint16_t>();
	new_condition.veins_point = condition["veins_point"].cast<std::array<std::uint16_t,14>>();
	new_condition.veins_amount = condition["veins_amount"].cast<std::array<std::uint64_t,14>>();
	set_need_veins(new_condition);
	std::vector<ConditionRef> children;
	for(auto child : condition["planets"].cast<py::list>())
		children.push_back(add_planet_condition(child.cast<py::dict>()));
	new_condition.planets = add_references(*this, children);
	new_condition.level = 1;
	for(std::size_t i = 0;i < new_condition.veins_point.size();++i) {
		if(new_condition.veins_point[i] != 0 || new_condition.veins_amount[i] != 0)
			new_condition.level = 3;
	}
	for(const ConditionRef& planet_ref : children)
		new_condition.level = std::max(new_condition.level,get_ref_level(*this,planet_ref));
	return add_star_condition(new_condition);
}

ConditionRef ConditionManager::add_star_condition(const StarCondition& condition) {
	return append_condition(stars,ConditionType::Star,condition);
}

ConditionRef ConditionManager::add_planet_condition(const py::dict& condition) {
	PlanetCondition new_condition;
	new_condition.type = condition["type"].cast<std::uint32_t>();
	const int liquid = condition["liquid"].cast<int>();
	if(liquid < 0 || liquid > 2)
		throw std::invalid_argument("liquid 必须为 0、1 或 2");
	new_condition.type &= liquid_masks[liquid];
	new_condition.satisfy_num = condition["satisfy_num"].cast<std::uint16_t>();
	new_condition.dsp_level = condition["dsp_level"].cast<std::uint8_t>();
	new_condition.singularity = condition["singularity"].cast<std::uint8_t>();
	new_condition.veins_point = condition["veins_point"].cast<std::array<std::uint16_t,14>>();
	new_condition.veins_amount = condition["veins_amount"].cast<std::array<std::uint64_t,14>>();
	set_need_veins(new_condition);
	std::vector<ConditionRef> children;
	for(auto child : condition["moons"].cast<py::list>())
		children.push_back(add_planet_condition(child.cast<py::dict>()));
	new_condition.moons = add_references(*this, children);
	new_condition.level = 2;
	for(std::size_t i = 0;i < new_condition.veins_point.size();++i) {
		if(new_condition.veins_point[i] != 0 || new_condition.veins_amount[i] != 0)
			new_condition.level = 3;
	}
	for(const ConditionRef& moon_ref : children)
		new_condition.level = std::max(new_condition.level,get_ref_level(*this,moon_ref));
	return add_planet_condition(new_condition);
}

ConditionRef ConditionManager::add_planet_condition(const PlanetCondition& condition) {
	return append_condition(planets,ConditionType::Planet,condition);
}

ConditionRef ConditionManager::add_bond_condition(const py::dict& condition) {
	BondCondition new_condition;
	new_condition.distance = condition["distance"].cast<double>();
	new_condition.satisfy_num = condition["satisfy_num"].cast<int>();
	const auto endpoint = [&](const char* key) -> ConditionRef {
		const py::dict value = condition[key].cast<py::dict>();
		const int type = value["type"].cast<int>();
		if(type != static_cast<int>(ConditionType::Star) && type != static_cast<int>(ConditionType::Planet))
			throw std::invalid_argument("绑定端点必须为恒星或行星条件");
		return handle_condition_dict(value);
	};
	new_condition.con1 = endpoint("con1");
	new_condition.con2 = endpoint("con2");
	new_condition.level = std::max(get_ref_level(*this,new_condition.con1),get_ref_level(*this,new_condition.con2));
	new_condition.level = std::max(new_condition.level,static_cast<std::uint8_t>(2));
	return add_bond_condition(new_condition);
}

ConditionRef ConditionManager::add_bond_condition(const BondCondition& condition) {
	return append_condition(bonds,ConditionType::Bond,condition);
}

ConditionRef ConditionManager::add_logic_condition(const py::dict& condition) {
	LogicCondition new_condition;
	new_condition.satisfy_num = condition["satisfy_num"].cast<std::uint16_t>();
	const py::list sub_conditions = condition["children"].cast<py::list>();
	if(sub_conditions.empty() && new_condition.satisfy_num != 0)
		throw std::invalid_argument("空逻辑节点的 satisfy_num 必须为 0");
	if(new_condition.satisfy_num == 0 && sub_conditions.size() > 1)
		throw std::invalid_argument("satisfy_num 为 0 时逻辑节点最多有一个子条件");
	std::vector<ConditionRef> children;
	for(auto child : sub_conditions)
		children.push_back(handle_condition_dict(child.cast<py::dict>()));
	const RefRange range = add_references(*this, children);
	new_condition.children_begin = range.begin;
	new_condition.children_count = range.count;
	return add_logic_condition(new_condition);
}

ConditionRef ConditionManager::add_logic_condition(const LogicCondition& condition) {
	return append_condition(logic_nodes,ConditionType::Logic,condition);
}
