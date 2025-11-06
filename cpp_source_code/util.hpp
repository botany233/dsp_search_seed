// Original 2022 Copyright https://github.com/crazyyao0.
#pragma once

#include <math.h>
#include <immintrin.h>

template<class T>
class Math_s
{
public:
	const T PI = (T)3.141592653589793238463;
	inline int CeilToInt(T a) const {
		return (int)ceil(a);
	}
	inline int RoundToInt(T a) const {
		return (int)round(a);
	}
	inline T Round(T a) const {
		return (T)round(a);
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
	inline T Acos(T a) const {
		return (T)acos(a);
	}
};

const Math_s<float> Mathf;
const Math_s<double> Math;

struct Vector2 {
	float x = 0.0f;
	float y = 0.0f;
};

class VectorLF3 {
public:
	static VectorLF3 zero() {
		return VectorLF3{0.0f,0.0f,0.0f};
	};
	double x;
	double y;
	double z;

	VectorLF3()
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	VectorLF3(double x,double y,double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	double sqrMagnitude() const {
		return x * x + y * y + z * z;
	}
	double magnitude() const {
		return sqrt(x * x + y * y + z * z);
	}
	VectorLF3 normalized() const {
		double num = x * x + y * y + z * z;
		if(num < 1E-34)
			return VectorLF3{0,0,0};
		double num2 = sqrt(num);
		return VectorLF3{x / num2,y / num2,z / num2};
	}
	VectorLF3 operator *(VectorLF3& rhs) const
	{
		return VectorLF3{x * rhs.x,y * rhs.y,z * rhs.z};
	}
	VectorLF3 operator *(double s) const
	{
		return VectorLF3{x * s,y * s,z * s};
	}

	VectorLF3 operator - (VectorLF3& rhs) const
	{
		return VectorLF3{x - rhs.x,y - rhs.y,z - rhs.z};
	}

	VectorLF3 operator + (VectorLF3& rhs) const
	{
		return VectorLF3{x + rhs.x,y + rhs.y,z + rhs.z};
	}
};

class DotNet35Random
{
public:
	int seedArray[56];
	int inext = 0;
	int inextp = 31;

	DotNet35Random(int seed)
	{
		int num = 161803398 - std::abs(seed);
		seedArray[55] = num;
		int num2 = 1;
		for(int i = 1; i < 55; i++)
		{
			int num3 = 21 * i % 55;
			seedArray[num3] = num2;
			num2 = num - num2;
			if(num2 < 0)
				num2 += 2147483647;
			num = seedArray[num3];
		}

		#ifdef SUPPORT_AVX2
		__m256i const offset = _mm256_setr_epi32(7,0,1,2,3,4,5,6);
		__m256i r0 = _mm256_loadu_si256((__m256i*)seedArray);
		__m256i r1 = _mm256_loadu_si256((__m256i*)(seedArray + 8));
		__m256i r2 = _mm256_loadu_si256((__m256i*)(seedArray + 16));
		__m256i r3 = _mm256_loadu_si256((__m256i*)(seedArray + 24));
		__m256i r4 = _mm256_loadu_si256((__m256i*)(seedArray + 32));
		__m256i r5 = _mm256_loadu_si256((__m256i*)(seedArray + 40));
		__m256i r6 = _mm256_loadu_si256((__m256i*)(seedArray + 48));
		for(int j = 1; j < 5; j++)
		{
			__m256i r7 = _mm256_permutevar8x32_epi32(r4,offset);
			r0 = _mm256_sub_epi32(r0,r7);
			r7 = _mm256_srai_epi32(r0,31);
			r7 = _mm256_srli_epi32(r7,1);
			r0 = _mm256_add_epi32(r0,r7);

			r7 = _mm256_permutevar8x32_epi32(r5,offset);
			r7 = _mm256_insert_epi32(r7,_mm256_extract_epi32(r4,7),0);
			r1 = _mm256_sub_epi32(r1,r7);
			r7 = _mm256_srai_epi32(r1,31);
			r7 = _mm256_srli_epi32(r7,1);
			r1 = _mm256_add_epi32(r1,r7);

			r7 = _mm256_permutevar8x32_epi32(r6,offset);
			r7 = _mm256_insert_epi32(r7,_mm256_extract_epi32(r5,7),0);
			r2 = _mm256_sub_epi32(r2,r7);
			r7 = _mm256_srai_epi32(r2,31);
			r7 = _mm256_srli_epi32(r7,1);
			r2 = _mm256_add_epi32(r2,r7);

			r7 = _mm256_insert_epi32(r0,_mm256_extract_epi32(r6,7),0);
			r3 = _mm256_sub_epi32(r3,r7);
			r7 = _mm256_srai_epi32(r3,31);
			r7 = _mm256_srli_epi32(r7,1);
			r3 = _mm256_add_epi32(r3,r7);

			r4 = _mm256_sub_epi32(r4,r1);
			r7 = _mm256_srai_epi32(r4,31);
			r7 = _mm256_srli_epi32(r7,1);
			r4 = _mm256_add_epi32(r4,r7);

			r5 = _mm256_sub_epi32(r5,r2);
			r7 = _mm256_srai_epi32(r5,31);
			r7 = _mm256_srli_epi32(r7,1);
			r5 = _mm256_add_epi32(r5,r7);

			r6 = _mm256_sub_epi32(r6,r3);
			r7 = _mm256_srai_epi32(r6,31);
			r7 = _mm256_srli_epi32(r7,1);
			r6 = _mm256_add_epi32(r6,r7);
		}
		_mm256_storeu_si256((__m256i*)seedArray,r0);
		_mm256_storeu_si256((__m256i*)(seedArray + 8),r1);
		_mm256_storeu_si256((__m256i*)(seedArray + 16),r2);
		_mm256_storeu_si256((__m256i*)(seedArray + 24),r3);
		_mm256_storeu_si256((__m256i*)(seedArray + 32),r4);
		_mm256_storeu_si256((__m256i*)(seedArray + 40),r5);
		_mm256_storeu_si256((__m256i*)(seedArray + 48),r6);
		#else
		for(int j = 1; j < 5; j++)
		{
			for(int k = 1; k < 56; k++)
			{
				seedArray[k] -= seedArray[1 + (k + 30) % 55];
				if(seedArray[k] < 0)
				{
					seedArray[k] += 2147483647;
				}
			}
		}
		#endif
	}

	double NextDouble()
	{
		return Sample();
	}

	double Sample()
	{
		if(++inext >= 56)
			inext = 1;
		if(++inextp >= 56)
			inextp = 1;
		int num = seedArray[inext] - seedArray[inextp];
		if(num < 0)
			num += 2147483647;
		seedArray[inext] = num;
		return (double)num * 4.6566128752457969E-10;
	}

	int Next()
	{
		return (int)(Sample() * 2147483647.0);
	}

	int Next(int maxValue)
	{
		return (int)(Sample() * (double)maxValue);
	}

	int Next(int minValue,int maxValue)
	{
		unsigned int num = (unsigned int)(maxValue - minValue);
		if(num <= 1)
			return minValue;
		return (int)((unsigned int)(Sample() * (double)num) + minValue);
	}
};