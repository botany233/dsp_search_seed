#pragma once

#include <iostream>
#include <cmath>
#include "util.hpp"

class Vector3 {
public:
	float x,y,z;

	// 构造函数
	Vector3() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	};

	Vector3(float x,float y,float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	};

	// 拷贝构造函数
	Vector3(const Vector3& other) {
		x = other.x;
		y = other.y;
		z = other.z;
	};

	// 赋值运算符
	Vector3& operator=(const Vector3& other);

	// 运算符重载
	Vector3 operator+(const Vector3& other) const;
	Vector3 operator-(const Vector3& other) const;
	Vector3 operator*(float scalar) const;
	Vector3 operator/(float scalar) const;
	Vector3& operator+=(const Vector3& other);
	Vector3& operator-=(const Vector3& other);
	Vector3& operator*=(float scalar);
	Vector3& operator/=(float scalar);
	bool operator==(const Vector3& other) const;
	bool operator!=(const Vector3& other) const;

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
		return Vector3(0.0f,0.0f,0.0f);
	};

	static Vector3 one() {
		return Vector3(1.0f,1.0f,1.0f);
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
};
