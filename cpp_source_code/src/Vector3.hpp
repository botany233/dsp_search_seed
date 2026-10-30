#pragma once

#include <bit>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include "util.hpp"
#include "VectorLF3.hpp"

class Vector3 {
public:
	float x,y,z;

	// 构造函数
	Vector3(): x(0.0f),y(0.0f),z(0.0f) {};

	Vector3(float x,float y,float z): x(x),y(y),z(z) {};

	Vector3(const Vector3& other): x(other.x),y(other.y),z(other.z) {};

	Vector3(const VectorLF3& other): x((float)other.x),y((float)other.y),z((float)other.z) {};

	// 成员函数
	float magnitude() const {
		return Magnitude(*this);
	};

	float sqrMagnitude() const {
		return static_cast<float>((static_cast<double>(x) * x + static_cast<double>(y) * y) + static_cast<double>(z) * z);
	};

	void Normalize() {
		float num = Magnitude(*this);
		if(num > 1e-05f) {
			*this = *this / num;
		} else {
			*this = zero();
		}
	}

	float dot(const Vector3& other) const {
		return Dot(*this,other);
	};

	Vector3 lerp(const Vector3& target,float t) const {
		t = Mathf.Clamp01(t); // 限制t在[0,1]范围内
		return Vector3(
			static_cast<float>(static_cast<double>(x) + (static_cast<double>(target.x) - x) * t),
			static_cast<float>(static_cast<double>(y) + (static_cast<double>(target.y) - y) * t),
			static_cast<float>(static_cast<double>(z) + (static_cast<double>(target.z) - z) * t)
		);
	};

	// 静态方法
	static float dot(const Vector3& a,const Vector3& b) {
		return Dot(a,b);
	}

	static Vector3 Cross(const Vector3& a,const Vector3& b) {
		return Vector3(
			static_cast<float>(static_cast<double>(a.y) * b.z - static_cast<double>(a.z) * b.y),
			static_cast<float>(static_cast<double>(a.z) * b.x - static_cast<double>(a.x) * b.z),
			static_cast<float>(static_cast<double>(a.x) * b.y - static_cast<double>(a.y) * b.x)
		);
	};

	static Vector3 lerp(const Vector3& a,const Vector3& b,float t) {
		return Vector3(
			static_cast<float>(static_cast<double>(a.x) + (static_cast<double>(b.x) - a.x) * t),
			static_cast<float>(static_cast<double>(a.y) + (static_cast<double>(b.y) - a.y) * t),
			static_cast<float>(static_cast<double>(a.z) + (static_cast<double>(b.z) - a.z) * t)
		);
	}

	static float Magnitude(const Vector3& vector) {
		// Match the intermediate precision observed in the game's Vector3.Magnitude.
		const double x = vector.x;
		const double y = vector.y;
		const double z = vector.z;
		return static_cast<float>(std::sqrt((x * x + y * y) + z * z));
	};

	static float Dot(const Vector3& a,const Vector3& b) {
		return static_cast<float>((static_cast<double>(a.x) * b.x + static_cast<double>(a.y) * b.y) + static_cast<double>(a.z) * b.z);
	};

	static Vector3 Normalize(const Vector3& value) {
		float num = Magnitude(value);
		if(num > 1e-05f) {
			return value / num;
		}
		return zero();
	};

	static Vector3 nativeLerp(const Vector3& a,const Vector3& b,float t) {
		return b * t + a * (1 - t);
	};

	static Vector3 Slerp(const Vector3& a,const Vector3& b,float t);

	static Vector3 zero() {
		return Vector3{0.0f,0.0f,0.0f};
	};
	static Vector3 one() {
		return Vector3{1.0f,1.0f,1.0f};
	};
	static Vector3 up() {
		return Vector3(0.0f,1.0f,0.0f);
	};
	static Vector3 down() {
		return Vector3(0.0f,-1.0f,0.0f);
	};
	static Vector3 right() {
		return Vector3(1.0f,0.0f,0.0f);
	};
	static Vector3 left() {
		return Vector3(-1.0f,0.0f,0.0f);
	};
	static Vector3 forward() {
		return Vector3(0.0f,0.0f,1.0f);
	};
	static Vector3 back() {
		return Vector3(0.0f,0.0f,-1.0f);
	};

	Vector3& operator=(const Vector3& other) {
		if(this != &other) {
			x = other.x;
			y = other.y;
			z = other.z;
		}
		return *this;
	};
	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x,y + other.y,z + other.z);
	};
	Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x,y - other.y,z - other.z);
	};
	Vector3 operator*(float scalar) const {
		return Vector3(x * scalar,y * scalar,z * scalar);
	};
	Vector3 operator/(float scalar) const {
		if(scalar != 0.0f) {
			return Vector3(x / scalar,y / scalar,z / scalar);
		} else {
			return Vector3(0.0f,0.0f,0.0f);
		}
	};
	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	};
	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	};
	Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	};
	Vector3& operator/=(float scalar) {
		if(scalar != 0.0f) {
			x /= scalar;
			y /= scalar;
			z /= scalar;
		}
		return *this;
	};
	bool operator==(const Vector3& other) const {
		const float dx = x - other.x,dy = y - other.y,dz = z - other.z;
		return (static_cast<double>(dx) * dx + static_cast<double>(dy) * dy) + static_cast<double>(dz) * dz < static_cast<double>(9.99999944E-11f);
	};
	bool operator!=(const Vector3& other) const {
		return !(*this == other);
	};
};

inline VectorLF3::VectorLF3(const Vector3& other): x((double)other.x),y((double)other.y),z((double)other.z) {};
