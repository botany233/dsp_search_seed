#pragma once
#include "condition_struct.hpp"
#include "data_struct.hpp"

#include "astro_class.hpp"
#include "src/max_flow.hpp"

// 每个搜索线程独占；跨种子保留缓冲容量，不保留结果或候选对象引用。
struct WorkSpace {
	GalaxyClassSimple galaxy;
	ResultManager results{0};
	std::vector<int> vein_candidates;
	std::vector<VectorLF3> bond_positions1;
	std::vector<VectorLF3> bond_positions2;
	MaxFlowGraph bond_graph{0};
};

// quick 使用资源上界，否则按需生成真实资源；根逻辑确定后提前返回。
// 条件在搜索期间只读，工作区仅由当前线程持有，引用不得跨种子保存。
bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick);
bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick,WorkSpace& workspace);
// 各级均从指定核心条件起点循环检查；结果索引不随遍历位置变化。
bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick,WorkSpace& workspace,std::size_t start_index);
