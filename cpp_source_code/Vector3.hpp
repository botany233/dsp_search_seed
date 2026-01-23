#pragma once

#include <cmath>

#include "util.hpp"
#include "VectorLF3.hpp"

class Vector3 {
public:
	float x,y,z;

	// 构造函数
	Vector3() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	};

	Vector3(float x,float y,float z): x(x),y(y),z(z) {};

	Vector3(const Vector3& other): x(other.x),y(other.y),z(other.z) {};

	Vector3(const VectorLF3& other): x((float)other.x),y((float)other.y),z((float)other.z) {};

	// 成员函数
	float magnitude() const {
		return std::sqrt(x * x + y * y + z * z);
	};

	float sqrMagnitude() const {
		return x * x + y * y + z * z;
	};

	void Normalize() {
		float num = magnitude();
		if(num > 1e-05f) {
			x /= num;
			y /= num;
			z /= num;
		} else {
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
	}

	float dot(const Vector3& other) const {
		return x * other.x + y * other.y + z * other.z;
	};

	Vector3 lerp(const Vector3& target,float t) const {
		t = Mathf.Clamp01(t); // 限制t在[0,1]范围内
		return Vector3(
			x + (target.x - x) * t,
			y + (target.y - y) * t,
			z + (target.z - z) * t
		);
	};

	// 静态方法
	static Vector3 Cross(const Vector3& a,const Vector3& b) {
		return Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	};

	static Vector3 Slerp(const Vector3& a,const Vector3& b,float t) {
		Vector3 startNormal = Normalize(a);
		Vector3 endNormal = Normalize(b);
		float dot = Vector3::Dot(startNormal,endNormal);
		dot = Mathf.Clamp(dot,-1.0f,1.0f);
		float theta = Mathf.Acos(dot) * t;
		Vector3 RelativeVec = endNormal - startNormal * dot;
		RelativeVec.Normalize();
		Vector3 result = (startNormal * Mathf.Cos(theta) + RelativeVec * Mathf.Sin(theta));
		float length = a.magnitude() * (1 - t) + b.magnitude() * t;
		return result * length;
	};

	static float Magnitude(const Vector3& vector) {
		return (float)std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	};

	static float Dot(const Vector3& a,const Vector3& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	};

	static Vector3 Normalize(const Vector3& value) {
		float num = Magnitude(value);
		if(num > 1e-05f) {
			return Vector3(value.x / num,value.y / num,value.z / num);
		}
		return zero();
	};
	
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
		const float epsilon = 1e-6f;
		return (std::abs(x - other.x) < epsilon) &&
			(std::abs(y - other.y) < epsilon) &&
			(std::abs(z - other.z) < epsilon);
	};
	bool operator!=(const Vector3& other) const {
		return !(*this == other);
	};
};

inline VectorLF3::VectorLF3(const Vector3& other) : x((double)other.x), y((double)other.y), z((double)other.z) {};
