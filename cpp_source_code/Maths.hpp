#pragma once

#include "Vector3.hpp"
#include "VectorLF3.hpp"
#include "quaternion.hpp"

class Maths
{
public:
	static double Clamp(double val,double min,double max)
	{
		if(val < min)
		{
			return min;
		}
		if(val > max)
		{
			return max;
		}
		return val;
	}

	static double Clamp01(double val)
	{
		if(val < 0.0)
		{
			return 0.0;
		}
		if(val > 1.0)
		{
			return 1.0;
		}
		return val;
	}

	static double Levelize(double f,double level = 1.0,double offset = 0.0) {
		f = f / level - offset;
		double num = std::floor(f);
		double num2 = f - num;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		f = num + num2;
		f = (f + offset) * level;
		return f;
	}

	static double Levelize2(double f,double level = 1.0,double offset = 0.0) {
		f = f / level - offset;
		double num = std::floor(f);
		double num2 = f - num;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		f = num + num2;
		f = (f + offset) * level;
		return f;
	}

	static double Levelize3(double f,double level = 1.0,double offset = 0.0) {
		f = f / level - offset;
		double num = std::floor(f);
		double num2 = f - num;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		f = num + num2;
		f = (f + offset) * level;
		return f;
	}

	static double Levelize4(double f,double level = 1.0,double offset = 0.0) {
		f = f / level - offset;
		double num = std::floor(f);
		double num2 = f - num;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		num2 = (3.0 - num2 - num2) * num2 * num2;
		f = num + num2;
		f = (f + offset) * level;
		return f;
	}

	static Vector3 QRotate(const Quaternion& q,Vector3 v) {
		v.x *= 2.0f;
		v.y *= 2.0f;
		v.z *= 2.0f;
		float num = q.w * q.w - 0.5f;
		float num2 = q.x * v.x + q.y * v.y + q.z * v.z;
		return Vector3{
			v.x * num + (q.y * v.z - q.z * v.y) * q.w + q.x * num2,
			v.y * num + (q.z * v.x - q.x * v.z) * q.w + q.y * num2,
			v.z * num + (q.x * v.y - q.y * v.x) * q.w + q.z * num2
		};
	}

	static VectorLF3 QInvRotateLF(const Quaternion& q,VectorLF3 v)
	{
		v.x *= 2.0;
		v.y *= 2.0;
		v.z *= 2.0;
		double num = (double)(q.w * q.w) - 0.5;
		double num2 = (double)q.x * v.x + (double)q.y * v.y + (double)q.z * v.z;
		return VectorLF3{
			v.x * num - ((double)q.y * v.z - (double)q.z * v.y) * (double)q.w + (double)q.x * num2,
			v.y * num - ((double)q.z * v.x - (double)q.x * v.z) * (double)q.w + (double)q.y * num2,
			v.z * num - ((double)q.x * v.y - (double)q.y * v.x) * (double)q.w + (double)q.z * num2
		};
	}
};
