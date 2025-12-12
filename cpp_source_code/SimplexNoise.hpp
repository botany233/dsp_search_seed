#pragma once
#include "util.hpp"
#include <cstdint>
//#include <cmath>
#include <immintrin.h>

using namespace std;

class Grad
{
public:
	double x,y,z,w;
	Grad(double x,double y,double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = 0.0;
	}

	Grad(double x,double y,double z,double w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

//struct Grad_fast
//{
//	double x,y,z;
//	Grad_fast(double x,double y,double z)
//	{
//		this->x = x;
//		this->y = y;
//		this->z = z;
//	}
//};

class SimplexNoise
{
public:
	const Grad grad3[12] = {
		Grad(1,1,0),Grad(-1,1,0),Grad(1,-1,0),Grad(-1,-1,0),
		Grad(1,0,1),Grad(-1,0,1),Grad(1,0,-1),Grad(-1,0,-1),
		Grad(0,1,1),Grad(0,-1,1),Grad(0,1,-1),Grad(0,-1,-1)
	};
	//const int8_t grad3[12][3] = {
	//	{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
	//	{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
	//	{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
	//	};
	short p[256] = {0};
	short perm[512] = {0};
	short permMod12[512] = {0};
	const double F3 = 1.0 / 3.0;
	const double G3 = 1.0 / 6.0;
	SimplexNoise(int seed) {
		InitNumber(seed);
	};

	void InitNumber(int seed) {
		for(int i = 0; i < 256; i++)
		{
			p[i] = (short)i;
		}
		DotNet35Random dotNet35Random = DotNet35Random(seed);
		for(int j = 0; j < 256; j++)
		{
			int num = dotNet35Random.Next(0,256);
			int num2 = p[j];
			p[j] = p[num];
			p[num] = (short)num2;
		}
		for(int k = 0; k < 512; k++)
		{
			perm[k] = p[k & 0xFF];
			permMod12[k] = (short)(perm[k] % 12);
		}
	};

	inline int fastfloor(double x)
	{
		return (int)std::floor(x);
	}

	inline double dot(const Grad& g,const double& x,const double& y,const double& z)
	{
		return g.x * x + g.y * y + g.z * z;
	}

	double Noise(double xin,double yin,double zin)
	{
		double num = (xin + yin + zin) * F3;
		int num2 = fastfloor(xin + num);
		int num3 = fastfloor(yin + num);
		int num4 = fastfloor(zin + num);
		double num5 = (double)(num2 + num3 + num4) * G3;
		double num6 = (double)num2 - num5;
		double num7 = (double)num3 - num5;
		double num8 = (double)num4 - num5;
		double num9 = xin - num6;
		double num10 = yin - num7;
		double num11 = zin - num8;
		int num12;
		int num13;
		int num14;
		int num15;
		int num16;
		int num17;
		if(num9 >= num10)
		{
			if(num10 >= num11)
			{
				num12 = 1;
				num13 = 0;
				num14 = 0;
				num15 = 1;
				num16 = 1;
				num17 = 0;
			} else if(num9 >= num11)
			{
				num12 = 1;
				num13 = 0;
				num14 = 0;
				num15 = 1;
				num16 = 0;
				num17 = 1;
			} else
			{
				num12 = 0;
				num13 = 0;
				num14 = 1;
				num15 = 1;
				num16 = 0;
				num17 = 1;
			}
		} else if(num10 < num11)
		{
			num12 = 0;
			num13 = 0;
			num14 = 1;
			num15 = 0;
			num16 = 1;
			num17 = 1;
		} else if(num9 < num11)
		{
			num12 = 0;
			num13 = 1;
			num14 = 0;
			num15 = 0;
			num16 = 1;
			num17 = 1;
		} else
		{
			num12 = 0;
			num13 = 1;
			num14 = 0;
			num15 = 1;
			num16 = 1;
			num17 = 0;
		}
		double num18 = num9 - (double)num12 + G3;
		double num19 = num10 - (double)num13 + G3;
		double num20 = num11 - (double)num14 + G3;
		double num21 = num9 - (double)num15 + 2.0 * G3;
		double num22 = num10 - (double)num16 + 2.0 * G3;
		double num23 = num11 - (double)num17 + 2.0 * G3;
		double num24 = num9 - 1.0 + 3.0 * G3;
		double num25 = num10 - 1.0 + 3.0 * G3;
		double num26 = num11 - 1.0 + 3.0 * G3;
		int num27 = num2 & 0xFF;
		int num28 = num3 & 0xFF;
		int num29 = num4 & 0xFF;
		int num30 = permMod12[num27 + perm[num28 + perm[num29]]];
		int num31 = permMod12[num27 + num12 + perm[num28 + num13 + perm[num29 + num14]]];
		int num32 = permMod12[num27 + num15 + perm[num28 + num16 + perm[num29 + num17]]];
		int num33 = permMod12[num27 + 1 + perm[num28 + 1 + perm[num29 + 1]]];
		
		#ifdef SUPPORT_AVX2
		__m256d x_vec = _mm256_set_pd(num24, num21, num18, num9);
		__m256d y_vec = _mm256_set_pd(num25, num22, num19, num10);
		__m256d z_vec = _mm256_set_pd(num26, num23, num20, num11);

		__m256d t_vec = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(x_vec, x_vec), _mm256_mul_pd(y_vec, y_vec)), _mm256_mul_pd(z_vec, z_vec));
		__m256d att_vec = _mm256_sub_pd(_mm256_set1_pd(0.6), t_vec);

		__m256d grad_x = _mm256_set_pd(grad3[num33].x, grad3[num32].x, grad3[num31].x, grad3[num30].x);
		__m256d grad_y = _mm256_set_pd(grad3[num33].y, grad3[num32].y, grad3[num31].y, grad3[num30].y);
		__m256d grad_z = _mm256_set_pd(grad3[num33].z, grad3[num32].z, grad3[num31].z, grad3[num30].z);

		__m256d dot_vec = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(grad_x, x_vec), _mm256_mul_pd(grad_y, y_vec)), _mm256_mul_pd(grad_z, z_vec));

		__m256d att_sq = _mm256_mul_pd(att_vec, att_vec);
		__m256d att_quad = _mm256_mul_pd(att_sq, att_sq);

		__m256d result_vec = _mm256_mul_pd(att_quad, dot_vec);

		__m256d mask = _mm256_cmp_pd(att_vec, _mm256_setzero_pd(), _CMP_LT_OQ);
		result_vec = _mm256_andnot_pd(mask, result_vec);

		double results[4];
		_mm256_storeu_pd(results, result_vec);
		double total = results[0] + results[1] + results[2] + results[3];
		#else
		double num34 = 0.6 - num9 * num9 - num10 * num10 - num11 * num11;
		double num35;
		if(num34 < 0.0)
		{
			num35 = 0.0;
		} else
		{
			num34 *= num34;
			num35 = num34 * num34 * dot(grad3[num30],num9,num10,num11);
		}
		double num36 = 0.6 - num18 * num18 - num19 * num19 - num20 * num20;
		double num37;
		if(num36 < 0.0)
		{
			num37 = 0.0;
		} else
		{
			num36 *= num36;
			num37 = num36 * num36 * dot(grad3[num31],num18,num19,num20);
		}
		double num38 = 0.6 - num21 * num21 - num22 * num22 - num23 * num23;
		double num39;
		if(num38 < 0.0)
		{
			num39 = 0.0;
		} else
		{
			num38 *= num38;
			num39 = num38 * num38 * dot(grad3[num32],num21,num22,num23);
		}
		double num40 = 0.6 - num24 * num24 - num25 * num25 - num26 * num26;
		double num41;
		if(num40 < 0.0)
		{
			num41 = 0.0;
		} else
		{
			num40 *= num40;
			num41 = num40 * num40 * dot(grad3[num33],num24,num25,num26);
		}
		double total = num35 + num37 + num39 + num41;
		#endif

		return 32.696434 * total;
	}
	
	double Noise3DFBM(double x,double y,double z,int nOctaves,double deltaAmp = 0.5,double deltaWLen = 2.0)
	{
		double num = 0.0;
		double num2 = 0.5;
		for(int i = 0; i < nOctaves; i++)
		{
			num += Noise(x,y,z) * num2;
			num2 *= deltaAmp;
			x *= deltaWLen;
			y *= deltaWLen;
			z *= deltaWLen;
		}
		return num;
	}

	double RidgedNoise(double x,double y,double z,int nOctaves,double deltaAmp = 0.5,double deltaWLen = 2.0,double initialAmp = 0.5)
	{
		double num = 0.0;
		double num2 = initialAmp;
		for(int i = 0; i < nOctaves; i++)
		{
			num += std::abs(Noise(x,y,z) * num2);
			num2 *= deltaAmp;
			x *= deltaWLen;
			y *= deltaWLen;
			z *= deltaWLen;
		}
		return num;
	}

	double Noise3DFBMInitialAmp(double x,double y,double z,int nOctaves,double deltaAmp = 0.5,double deltaWLen = 2.0,double initialAmp = 0.5)
	{
		double num = 0.0;
		double num2 = initialAmp;
		for(int i = 0; i < nOctaves; i++)
		{
			num += Noise(x,y,z) * num2;
			num2 *= deltaAmp;
			x *= deltaWLen;
			y *= deltaWLen;
			z *= deltaWLen;
		}
		return num;
	}
};
