#pragma once
#include "data_struct.hpp"

// check_level 为检查阶段上限：1 恒星，2 主题与结构，3 资源上界，4 真实资源。
// 四级使用独立判断函数；全部条件通过当前级别后，才进入下一级别。
// 条件可包含更高级需求；低级阶段只判断当前可确定的部分，最后统一应用 valid_state。
bool check_seed(const SeedStruct& seed,const GalaxyCondition& galaxy_condition,int check_level);
