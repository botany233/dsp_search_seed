// Original 2022 Copyright https://github.com/crazyyao0.
#pragma once

#include <cmath>

class Vector3;

class VectorLF3 {
public:
	double x,y,z;

	VectorLF3(): x(0.0),y(0.0),z(0.0) {};

	VectorLF3(double x,double y,double z): x(x),y(y),z(z) {};

	VectorLF3(const Vector3& other);

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

	static VectorLF3 zero() {
		return VectorLF3{0.0f,0.0f,0.0f};
	};

	VectorLF3 operator*(VectorLF3& rhs) const
	{
		return VectorLF3{x * rhs.x,y * rhs.y,z * rhs.z};
	}
	VectorLF3 operator*(double s) const
	{
		return VectorLF3{x * s,y * s,z * s};
	}
	VectorLF3 operator-(const VectorLF3& rhs) const
	{
		return VectorLF3{x - rhs.x,y - rhs.y,z - rhs.z};
	}
	VectorLF3 operator+(const VectorLF3& rhs) const
	{
		return VectorLF3{x + rhs.x,y + rhs.y,z + rhs.z};
	}
};
