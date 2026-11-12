#pragma once
#include "defines.hpp"

// 数量对应 veins_point，储量对应 veins_amount。
enum class VeinMetric : std::uint8_t {
	Point,
	Amount
};

// 返回 P(真实值/上界 > ratio)，范围为 [0,1]。
// vein_type 使用 EVeinType 的 1..14 编号，不是数组的 0..13 下标。
// 非法矿种、指标或 NaN 抛出 std::invalid_argument；允许无穷阈值。
// 数据为经验分布；0% 分位以比例 0 补足，具体边界见 docs/checkseed_opt.md。
double get_vein_exceed_probability(EVeinType vein_type, VeinMetric metric, double ratio);
