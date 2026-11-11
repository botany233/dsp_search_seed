#pragma once
#include "condition_struct.hpp"
#include "data_struct.hpp"

class GalaxyClassSimple;

// quick 使用资源上界，否则按需生成真实资源；根逻辑确定后提前返回。
// 条件在搜索期间只读，工作区仅由当前线程持有，引用不得跨种子保存。
bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick);
bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick,GalaxyClassSimple& workspace);
// 各级均从指定核心条件起点循环检查；结果索引不随遍历位置变化。
bool check_seed(const SeedStruct& seed,const ConditionManager& conditions,bool quick,GalaxyClassSimple& workspace,std::size_t start_index);
