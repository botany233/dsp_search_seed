#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

#include "data_struct.hpp"
#include "condition_struct.hpp"

namespace py = pybind11;

void do_init();
// 非空批次仅在 end_star_num - start_star_num > 1 时按 end_star_num - 1 缓存坐标。
std::vector<std::string> check_batch_c(int start_seed,int end_seed,int start_star_num,int end_star_num,uint8_t resource_index,
	const ConditionManager& galaxy_condition,bool quick);
