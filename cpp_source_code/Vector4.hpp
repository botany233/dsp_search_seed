#pragma once

#include <iostream>
#include <cmath>
#include "util.hpp"

class Vector4 {
public:
	static constexpr float kEpsilon = 1E-05f;
	float x,y,z,w;
	// 构造函数
	Vector4() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	};
	Vector4(float x,float y,float z) {
		this->x = x;
		this->y = y;
		this->z = z;
		w = 0.0f;
	};
	Vector4(float x,float y,float z,float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	};

	Vector4& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	};

	float magnitude() const {
		return std::sqrt(x * x + y * y + z * z + w * w);
	};

	void Normalize() {
		float num = magnitude();
		if(num > 1e-05f) {
			x /= num;
			y /= num;
			z /= num;
			w /= num;
		} else {
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
			w = 0.0f;
		}
	}
};
