// Original 2022 Copyright https://github.com/crazyyao0.
#pragma once

#include <math.h>
#include <cmath>

template<class T>
class Math_s
{
public:
	const T PI = (T)3.141592653589793;
	inline int CeilToInt(T a) const {
		return (int)ceil(a);
	}
	inline int RoundToInt(T a) const {
		return (int)std::nearbyint(a);
	}
	inline int FloorToInt(T a) const {
		return (int)floor(a);
	}
	inline T Round(T a) const {
		return (T)round(a);
	}
	inline T Sign(T a) const {
		return (a >= (T)0) ? (T)1 : (T)-1;
	}
	inline T Clamp(T a,T min,T max) const {
		if(a < min)return min;
		if(a > max)return max;
		return a;
	}
	inline T Clamp01(T a) const {
		if(a < 0)return 0;
		if(a > 1)return 1;
		return a;
	}
	inline T Lerp(T a,T b,T t) const {
		return a + (b - a) * Clamp01(t);
	}
	inline T Abs(T a) const {
		return (T)abs(a);
	}
	inline T Sqrt(T a) const {
		return (T)sqrt(a);
	}
	inline T Pow(T base,T exp) const {
		return (T)pow(base,exp);
	}
	inline T Sin(T a) const {
		return (T)sin(a);
	}
	inline T Cos(T a) const {
		return (T)cos(a);
	}
	inline T Ceil(T a) const {
		return (T)ceil(a);
	}
	inline T Log10(T v) const {
		return (T)log10(v);
	}
	inline T Log(T v) const {
		return (T)log(v);
	}
	inline T Min(T a,T b) const {
		return a < b ? a : b;
	}
	inline T Max(T a,T b) const {
		return a > b ? a : b;
	}
	inline T Asin(T a) const {
		return (T)asin(a);
	}
	inline T Acos(T a) const {
		return (T)acos(a);
	}
};

const Math_s<float> Mathf;
const Math_s<double> Math;
