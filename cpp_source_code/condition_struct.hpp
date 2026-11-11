#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>

namespace py = pybind11;

enum class EnumResult : std::uint8_t {
	Unknown = 0,
	False = 1,
	True = 2
};

// 通过稳定的 result_index 访问；只保存全星系核心条件的结果。
struct ResultManager {
	std::vector<EnumResult> results;

	explicit ResultManager(std::size_t core_condition_count);
	ResultManager(const ResultManager&) = delete;
	ResultManager& operator=(const ResultManager&) = delete;
};

// 复杂条件
enum class ConditionType : std::uint8_t {
	Invalid = 0,
	GalaxyMineral = 1,
	Star = 2,
	Planet = 3,
	Bond = 4,
	Logic = 5
};

// index 指向对应类型的条件池，而非 references。
struct ConditionRef {
	ConditionType type = ConditionType::Invalid;
	std::uint16_t index = 0;
	std::uint16_t result_index = std::numeric_limits<std::uint16_t>::max();
};

// 在主对象 references 中的连续片段；允许重用片段和重复引用。
struct RefRange {
	std::uint16_t begin = 0;
	std::uint16_t count = 0;
};

struct GalaxyMineralCondition {
	std::uint16_t need_veins = 0;
	bool need_veins_amount = false;
	std::array<std::uint16_t,14> veins_point{};
	std::array<std::uint64_t,14> veins_amount{};
};

// 核心属性隐式 AND；普通集合中 satisfy_num 为最低数量，零自动满足。
struct PlanetCondition {
	std::uint16_t need_veins = 0;
	bool need_veins_amount = false;
	std::uint32_t type = 0xFFFFFFFF;
	std::uint16_t satisfy_num = 1;
	std::uint8_t dsp_level = 0;
	std::uint8_t singularity = 0;
	std::uint8_t level = 0; //1-3
	std::array<std::uint16_t,14> veins_point{};
	std::array<std::uint64_t,14> veins_amount{};
	RefRange moons; // 仅 Planet，每项均在当前行星的卫星集合中满足。
};

struct StarCondition {
	std::uint16_t need_veins = 0;
	bool need_veins_amount = false;
	float distance = 1000.0f; // 距出生恒星的 LY 上限。
	float dyson_lumino = 0.0f; // 光度下限。
	std::uint32_t type = 0xFFFFFFFF;
	std::uint16_t satisfy_num = 1;
	std::uint8_t level = 0; //1-3
	std::array<std::uint16_t,14> veins_point{};
	std::array<std::uint64_t,14> veins_amount{};
	RefRange planets; // 仅 Planet，每项均在当前恒星的行星集合中满足。
};

struct BondCondition {
	double distance = 1000.0; // 两端之间的 LY 上限。
	int satisfy_num = 0; // 所需绑定数，非正自动满足。
	std::uint8_t level = 0; //1-3
	// 仅 Star/Planet；端点核心条件的 satisfy_num 表示每个端点容量。
	// 端点对象内部的行星/卫星数量要求仍按普通集合规则解释。
	ConditionRef con1;
	ConditionRef con2;
};

struct LogicCondition {
	std::uint16_t children_begin = 0;
	std::uint16_t children_count = 0; // 重复出现分别计数；空节点仅允许 K=0。
	// K>0 为至少 K 个真；K=0 时，空节点为恒真，单子项为非门。
	// 子条件取完整结果；其中 Star/Planet 在全星系范围统计。
	std::uint16_t satisfy_num = 1;
};

// 仅共享定义，不共享候选对象或上下文结果。追加和原位填写保持索引；
// 插入、删除及排序可能使索引失效。构建并校验后供消费者只读使用。
class ConditionManager {
public:
	std::vector<GalaxyMineralCondition> galaxy_minerals;
	std::vector<StarCondition> stars;
	std::vector<PlanetCondition> planets;
	std::vector<BondCondition> bonds;
	std::vector<LogicCondition> logic_nodes;
	std::vector<ConditionRef> references;
	std::vector<ConditionRef> core_conditions;
	ConditionRef root;
	std::uint16_t all_need_veins = 0;

	// 调用方须已初始化 Python，并在构造期间持有 GIL。
	explicit ConditionManager(const std::string& galaxy_condition_json);
	explicit ConditionManager(const py::dict& galaxy_condition_dict);

	// 每个 core_conditions 条目保存完整结果；局部条件在各自上下文求值。
	ResultManager create_result_manager() const;
	EnumResult evaluate_root(const ResultManager& result_manager) const;
	// 仅允许在没有搜索线程读取时旋转核心列表；保留所有结果索引。
	void rotate_core_conditions(std::size_t start_index);

private:
	ConditionRef add_galaxy_mineral_condition(const GalaxyMineralCondition& condition);
	ConditionRef add_galaxy_mineral_condition(const py::dict& condition);
	ConditionRef add_star_condition(const StarCondition& condition);
	ConditionRef add_star_condition(const py::dict& condition);
	ConditionRef add_planet_condition(const PlanetCondition& condition);
	ConditionRef add_planet_condition(const py::dict& condition);
	ConditionRef add_bond_condition(const BondCondition& condition);
	ConditionRef add_bond_condition(const py::dict& condition);
	ConditionRef add_logic_condition(const LogicCondition& condition);
	ConditionRef add_logic_condition(const py::dict& condition);
	ConditionRef handle_condition_dict(const py::dict& condition_dict);
};
