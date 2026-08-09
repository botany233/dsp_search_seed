#pragma once

#include <cstddef>
#include <cstdint>
#include <immintrin.h>

#include "DotNet35Random.hpp"

using namespace std;

#ifndef SUPPORT_AVX2
class Grad
{
public:
	double x,y,z;
	constexpr Grad(double x,double y,double z): x(x),y(y),z(z) {}
};
#endif

class SimplexNoise
{
public:
	int perm[512] = {0};
	int permMod12[512] = {0};
protected:
	static constexpr double F3 = 1.0 / 3.0;
	static constexpr double G3 = 1.0 / 6.0;

	#ifndef SUPPORT_AVX2
	static constexpr Grad grad3[12] = {
		Grad(1,1,0),Grad(-1,1,0),Grad(1,-1,0),Grad(-1,-1,0),
		Grad(1,0,1),Grad(-1,0,1),Grad(1,0,-1),Grad(-1,0,-1),
		Grad(0,1,1),Grad(0,-1,1),Grad(0,1,-1),Grad(0,-1,-1)
	};

	inline double dot(const Grad& g,const double& x,const double& y,const double& z) const
	{
		return g.x * x + g.y * y + g.z * z;
	}
	#endif

	void Init(int seed) {
		short p[256] = {0};
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

	static int fastfloor(double x){
		return (int)std::floor(x);
	}

	#ifdef SUPPORT_AVX2
	static __m256d Contribution(__m256d x,__m256d y,__m256d z,__m128i gradientIndices32) {
		__m256d squaredLength = _mm256_mul_pd(z,z);
		squaredLength = _mm256_fmadd_pd(y,y,squaredLength);
		squaredLength = _mm256_fmadd_pd(x,x,squaredLength);
		__m256d attenuation = _mm256_sub_pd(_mm256_set1_pd(0.6),squaredLength);

		__m256i gradientIndices = _mm256_cvtepi32_epi64(gradientIndices32);
		__m256i useX = _mm256_cmpgt_epi64(_mm256_set1_epi64x(8),gradientIndices);
		__m256i useY = _mm256_cmpgt_epi64(_mm256_set1_epi64x(4),gradientIndices);
		__m256d gradientA = _mm256_blendv_pd(y,x,_mm256_castsi256_pd(useX));
		__m256d gradientB = _mm256_blendv_pd(z,y,_mm256_castsi256_pd(useY));

		__m256i signA = _mm256_slli_epi64(gradientIndices,63);
		__m256i signB = _mm256_slli_epi64(
			_mm256_and_si256(gradientIndices,_mm256_set1_epi64x(2)),62);
		gradientA = _mm256_xor_pd(gradientA,_mm256_castsi256_pd(signA));
		gradientB = _mm256_xor_pd(gradientB,_mm256_castsi256_pd(signB));
		__m256d gradientDot = _mm256_add_pd(gradientA,gradientB);

		attenuation = _mm256_max_pd(_mm256_setzero_pd(),attenuation);
		attenuation = _mm256_mul_pd(attenuation,attenuation);
		attenuation = _mm256_mul_pd(attenuation,attenuation);
		return _mm256_mul_pd(attenuation,gradientDot);
	}

	__m256d Noise4(__m256d xin,__m256d yin,__m256d zin) const {
		__m256d skew = _mm256_mul_pd(
			_mm256_add_pd(_mm256_add_pd(xin,yin),zin),
			_mm256_set1_pd(F3));
		__m256d iFloor = _mm256_floor_pd(_mm256_add_pd(xin,skew));
		__m256d jFloor = _mm256_floor_pd(_mm256_add_pd(yin,skew));
		__m256d kFloor = _mm256_floor_pd(_mm256_add_pd(zin,skew));
		__m128i i = _mm256_cvttpd_epi32(iFloor);
		__m128i j = _mm256_cvttpd_epi32(jFloor);
		__m128i k = _mm256_cvttpd_epi32(kFloor);

		__m128i latticeSum = _mm_add_epi32(_mm_add_epi32(i,j),k);
		__m256d unskew = _mm256_mul_pd(
			_mm256_cvtepi32_pd(latticeSum),
			_mm256_set1_pd(G3));
		__m256d x0 = _mm256_sub_pd(xin,_mm256_sub_pd(_mm256_cvtepi32_pd(i),unskew));
		__m256d y0 = _mm256_sub_pd(yin,_mm256_sub_pd(_mm256_cvtepi32_pd(j),unskew));
		__m256d z0 = _mm256_sub_pd(zin,_mm256_sub_pd(_mm256_cvtepi32_pd(k),unskew));

		__m256d xGeY = _mm256_cmp_pd(x0,y0,_CMP_GE_OQ);
		__m256d xGeZ = _mm256_cmp_pd(x0,z0,_CMP_GE_OQ);
		__m256d yGtX = _mm256_cmp_pd(y0,x0,_CMP_GT_OQ);
		__m256d yGeZ = _mm256_cmp_pd(y0,z0,_CMP_GE_OQ);
		__m256d one = _mm256_set1_pd(1.0);
		__m128i i1 = _mm256_cvttpd_epi32(
			_mm256_and_pd(_mm256_and_pd(xGeY,xGeZ),one));
		__m128i j1 = _mm256_cvttpd_epi32(
			_mm256_and_pd(_mm256_and_pd(yGtX,yGeZ),one));
		__m128i i2 = _mm256_cvttpd_epi32(
			_mm256_and_pd(_mm256_or_pd(xGeY,xGeZ),one));
		__m128i j2 = _mm256_cvttpd_epi32(
			_mm256_and_pd(_mm256_or_pd(yGtX,yGeZ),one));
		__m128i oneInt = _mm_set1_epi32(1);
		__m128i k1 = _mm_sub_epi32(oneInt,_mm_add_epi32(i1,j1));
		__m128i k2 = _mm_sub_epi32(_mm_set1_epi32(2),_mm_add_epi32(i2,j2));

		__m128i mask255 = _mm_set1_epi32(0xFF);
		__m128i ii = _mm_and_si128(i,mask255);
		__m128i jj = _mm_and_si128(j,mask255);
		__m128i kk = _mm_and_si128(k,mask255);

		// Process the same hash stage for all four corners before advancing.
		__m128i hash0 = _mm_i32gather_epi32(perm,kk,4);
		__m128i hash1 = _mm_i32gather_epi32(perm,_mm_add_epi32(kk,k1),4);
		__m128i hash2 = _mm_i32gather_epi32(perm,_mm_add_epi32(kk,k2),4);
		__m128i hash3 = _mm_i32gather_epi32(perm,_mm_add_epi32(kk,oneInt),4);

		hash0 = _mm_i32gather_epi32(perm,_mm_add_epi32(jj,hash0),4);
		hash1 = _mm_i32gather_epi32(perm,_mm_add_epi32(_mm_add_epi32(jj,j1),hash1),4);
		hash2 = _mm_i32gather_epi32(perm,_mm_add_epi32(_mm_add_epi32(jj,j2),hash2),4);
		hash3 = _mm_i32gather_epi32(perm,_mm_add_epi32(_mm_add_epi32(jj,oneInt),hash3),4);

		__m128i gradient0 = _mm_i32gather_epi32(
			permMod12,_mm_add_epi32(ii,hash0),4);
		__m128i gradient1 = _mm_i32gather_epi32(
			permMod12,_mm_add_epi32(_mm_add_epi32(ii,i1),hash1),4);
		__m128i gradient2 = _mm_i32gather_epi32(
			permMod12,_mm_add_epi32(_mm_add_epi32(ii,i2),hash2),4);
		__m128i gradient3 = _mm_i32gather_epi32(
			permMod12,_mm_add_epi32(_mm_add_epi32(ii,oneInt),hash3),4);

		__m256d result = Contribution(x0,y0,z0,gradient0);
		__m256d g1 = _mm256_set1_pd(G3);
		__m256d x1 = _mm256_add_pd(_mm256_sub_pd(x0,_mm256_cvtepi32_pd(i1)),g1);
		__m256d y1 = _mm256_add_pd(_mm256_sub_pd(y0,_mm256_cvtepi32_pd(j1)),g1);
		__m256d z1 = _mm256_add_pd(_mm256_sub_pd(z0,_mm256_cvtepi32_pd(k1)),g1);
		result = _mm256_add_pd(result,Contribution(x1,y1,z1,gradient1));

		__m256d g2 = _mm256_set1_pd(2.0 * G3);
		__m256d x2 = _mm256_add_pd(_mm256_sub_pd(x0,_mm256_cvtepi32_pd(i2)),g2);
		__m256d y2 = _mm256_add_pd(_mm256_sub_pd(y0,_mm256_cvtepi32_pd(j2)),g2);
		__m256d z2 = _mm256_add_pd(_mm256_sub_pd(z0,_mm256_cvtepi32_pd(k2)),g2);
		result = _mm256_add_pd(result,Contribution(x2,y2,z2,gradient2));

		__m256d half = _mm256_set1_pd(0.5);
		result = _mm256_add_pd(result,Contribution(
			_mm256_sub_pd(x0,half),
			_mm256_sub_pd(y0,half),
			_mm256_sub_pd(z0,half),
			gradient3));

		return _mm256_mul_pd(result,_mm256_set1_pd(32.696434));
	}
	#endif

public:
	SimplexNoise() {};

	SimplexNoise(int seed) {
		Init(seed);
	};

	#ifdef SUPPORT_AVX2
	double Noise(double xin,double yin,double zin) const
	{
		double skew = (xin + yin + zin) * F3;
		int i = fastfloor(xin + skew);
		int j = fastfloor(yin + skew);
		int k = fastfloor(zin + skew);

		double unskew = (double)(i + j + k) * G3;
		double x0 = xin - ((double)i - unskew);
		double y0 = yin - ((double)j - unskew);
		double z0 = zin - ((double)k - unskew);

		// Rank comparisons preserve the original tie ordering without branches.
		int rankX = (int)(x0 >= y0) + (int)(x0 >= z0);
		int rankY = (int)(y0 > x0) + (int)(y0 >= z0);
		int rankZ = (int)(z0 > x0) + (int)(z0 > y0);

		int i1 = (int)(rankX == 2);
		int j1 = (int)(rankY == 2);
		int k1 = (int)(rankZ == 2);
		int i2 = (int)(rankX >= 1);
		int j2 = (int)(rankY >= 1);
		int k2 = (int)(rankZ >= 1);

		int ii = i & 0xFF;
		int jj = j & 0xFF;
		int kk = k & 0xFF;

		// Expose four independent hash chains to the out-of-order scheduler.
		int hash0 = perm[kk];
		int hash1 = perm[kk + k1];
		int hash2 = perm[kk + k2];
		int hash3 = perm[kk + 1];
		hash0 = perm[jj + hash0];
		hash1 = perm[jj + j1 + hash1];
		hash2 = perm[jj + j2 + hash2];
		hash3 = perm[jj + 1 + hash3];

		__m128i gradientIndices32 = _mm_setr_epi32(
			permMod12[ii + hash0],
			permMod12[ii + i1 + hash1],
			permMod12[ii + i2 + hash2],
			permMod12[ii + 1 + hash3]);

		__m128i iOffsets = _mm_setr_epi32(0,i1,i2,1);
		__m128i jOffsets = _mm_setr_epi32(0,j1,j2,1);
		__m128i kOffsets = _mm_setr_epi32(0,k1,k2,1);
		__m256d cornerUnskew = _mm256_setr_pd(0.0,G3,2.0 * G3,3.0 * G3);

		__m256d x = _mm256_add_pd(
			_mm256_sub_pd(_mm256_set1_pd(x0),_mm256_cvtepi32_pd(iOffsets)),
			cornerUnskew);
		__m256d y = _mm256_add_pd(
			_mm256_sub_pd(_mm256_set1_pd(y0),_mm256_cvtepi32_pd(jOffsets)),
			cornerUnskew);
		__m256d z = _mm256_add_pd(
			_mm256_sub_pd(_mm256_set1_pd(z0),_mm256_cvtepi32_pd(kOffsets)),
			cornerUnskew);

		__m256d squaredLength = _mm256_mul_pd(z,z);
		squaredLength = _mm256_fmadd_pd(y,y,squaredLength);
		squaredLength = _mm256_fmadd_pd(x,x,squaredLength);
		__m256d attenuation = _mm256_sub_pd(_mm256_set1_pd(0.6),squaredLength);

		// grad3[gi] always selects two coordinates; gi bits encode their signs.
		__m256i gradientIndices = _mm256_cvtepi32_epi64(gradientIndices32);
		__m256i useX = _mm256_cmpgt_epi64(_mm256_set1_epi64x(8),gradientIndices);
		__m256i useY = _mm256_cmpgt_epi64(_mm256_set1_epi64x(4),gradientIndices);
		__m256d gradientA = _mm256_blendv_pd(y,x,_mm256_castsi256_pd(useX));
		__m256d gradientB = _mm256_blendv_pd(z,y,_mm256_castsi256_pd(useY));

		__m256i signA = _mm256_slli_epi64(gradientIndices,63);
		__m256i signB = _mm256_slli_epi64(
			_mm256_and_si256(gradientIndices,_mm256_set1_epi64x(2)),62);
		gradientA = _mm256_xor_pd(gradientA,_mm256_castsi256_pd(signA));
		gradientB = _mm256_xor_pd(gradientB,_mm256_castsi256_pd(signB));
		__m256d gradientDot = _mm256_add_pd(gradientA,gradientB);

		attenuation = _mm256_max_pd(_mm256_setzero_pd(),attenuation);
		attenuation = _mm256_mul_pd(attenuation,attenuation);
		attenuation = _mm256_mul_pd(attenuation,attenuation);
		__m256d result = _mm256_mul_pd(attenuation,gradientDot);

		// Keep the same left-to-right reduction order as the scalar version.
		__m128d result01 = _mm256_castpd256_pd128(result);
		__m128d result1 = _mm_unpackhi_pd(result01,result01);
		__m128d total = _mm_add_sd(result01,result1);
		__m128d result23 = _mm256_extractf128_pd(result,1);
		total = _mm_add_sd(total,result23);
		__m128d result3 = _mm_unpackhi_pd(result23,result23);
		total = _mm_add_sd(total,result3);

		return 32.696434 * _mm_cvtsd_f64(total);
	}

	double Noise3DFBM(double x,double y,double z,int nOctaves,double deltaAmp = 0.5,double deltaWLen = 2.0,double initialAmp = 0.5) const {
		double total = 0.0;
		int octave = 0;

		while(octave < nOctaves)
		{
			int remaining = nOctaves - octave;
			if(remaining == 1)
			{
				total += Noise(x,y,z) * initialAmp;
				break;
			}

			// Generate lanes sequentially to match the scalar frequency progression.
			double x1 = x * deltaWLen;
			double y1 = y * deltaWLen;
			double z1 = z * deltaWLen;
			double x2 = x1 * deltaWLen;
			double y2 = y1 * deltaWLen;
			double z2 = z1 * deltaWLen;
			double x3 = x2 * deltaWLen;
			double y3 = y2 * deltaWLen;
			double z3 = z2 * deltaWLen;

			__m256d values = Noise4(
				_mm256_setr_pd(x,x1,x2,x3),
				_mm256_setr_pd(y,y1,y2,y3),
				_mm256_setr_pd(z,z1,z2,z3));

			double amplitude1 = initialAmp * deltaAmp;
			double amplitude2 = amplitude1 * deltaAmp;
			double amplitude3 = amplitude2 * deltaAmp;
			values = _mm256_mul_pd(values,_mm256_setr_pd(
				initialAmp,amplitude1,amplitude2,amplitude3));

			double octaveValues[4];
			_mm256_storeu_pd(octaveValues,values);
			int laneCount = (remaining < 4) ? remaining : 4;
			total += octaveValues[0];
			total += octaveValues[1];
			if(laneCount >= 3)
				total += octaveValues[2];
			if(laneCount == 4)
				total += octaveValues[3];

			if(laneCount < 4)
				break;

			x = x3 * deltaWLen;
			y = y3 * deltaWLen;
			z = z3 * deltaWLen;
			initialAmp = amplitude3 * deltaAmp;
			octave += 4;
		}

		return total;
	}

	double RidgedNoise(double x,double y,double z,int nOctaves,double deltaAmp = 0.5,double deltaWLen = 2.0,double initialAmp = 0.5) const
	{
		double total = 0.0;
		int octave = 0;

		while(octave < nOctaves)
		{
			int remaining = nOctaves - octave;
			if(remaining == 1)
			{
				total += std::abs(Noise(x,y,z) * initialAmp);
				break;
			}

			// Generate lanes sequentially to match the scalar frequency progression.
			double x1 = x * deltaWLen;
			double y1 = y * deltaWLen;
			double z1 = z * deltaWLen;
			double x2 = x1 * deltaWLen;
			double y2 = y1 * deltaWLen;
			double z2 = z1 * deltaWLen;
			double x3 = x2 * deltaWLen;
			double y3 = y2 * deltaWLen;
			double z3 = z2 * deltaWLen;

			__m256d values = Noise4(
				_mm256_setr_pd(x,x1,x2,x3),
				_mm256_setr_pd(y,y1,y2,y3),
				_mm256_setr_pd(z,z1,z2,z3));

			double amplitude1 = initialAmp * deltaAmp;
			double amplitude2 = amplitude1 * deltaAmp;
			double amplitude3 = amplitude2 * deltaAmp;
			values = _mm256_mul_pd(values,_mm256_setr_pd(
				initialAmp,amplitude1,amplitude2,amplitude3));
			values = _mm256_andnot_pd(_mm256_set1_pd(-0.0),values);

			double octaveValues[4];
			_mm256_storeu_pd(octaveValues,values);
			int laneCount = (remaining < 4) ? remaining : 4;
			total += octaveValues[0];
			total += octaveValues[1];
			if(laneCount >= 3)
				total += octaveValues[2];
			if(laneCount == 4)
				total += octaveValues[3];

			if(laneCount < 4)
				break;

			x = x3 * deltaWLen;
			y = y3 * deltaWLen;
			z = z3 * deltaWLen;
			initialAmp = amplitude3 * deltaAmp;
			octave += 4;
		}

		return total;
	}
	#else
	double Noise(double xin,double yin,double zin) const {
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

		int num12,num13,num14,num15,num16,num17;
		if(num9 >= num10) {
			if(num10 >= num11) {
				num12 = 1; num13 = 0; num14 = 0; num15 = 1; num16 = 1; num17 = 0;
			} else if(num9 >= num11) {
				num12 = 1; num13 = 0; num14 = 0; num15 = 1; num16 = 0; num17 = 1;
			} else {
				num12 = 0; num13 = 0; num14 = 1; num15 = 1; num16 = 0; num17 = 1;
			}
		} else if(num10 < num11) {
			num12 = 0; num13 = 0; num14 = 1; num15 = 0; num16 = 1; num17 = 1;
		} else if(num9 < num11) {
			num12 = 0; num13 = 1; num14 = 0; num15 = 0; num16 = 1; num17 = 1;
		} else {
			num12 = 0; num13 = 1; num14 = 0; num15 = 1; num16 = 1; num17 = 0;
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

		return 32.696434 * total;
	}

	double Noise3DFBM(double x,double y,double z,int nOctaves,double deltaAmp = 0.5,double deltaWLen = 2.0,double initialAmp = 0.5) const
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
	#endif
};
