#pragma once

#include <iostream>
#include <cmath>

#include "util.hpp"
#include "Vector3.hpp"

class Quaternion {
public:
	float x, y, z, w;
	// 构造函数
	Quaternion() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 1.0f;
	};
	
	Quaternion(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	};

	Quaternion operator*(const Quaternion& other) const {
		return Quaternion(
			w*other.x + x*other.w + y*other.z - z*other.y,
			w*other.y + y*other.w + z*other.x - x*other.z,
			w*other.z + z*other.w + x*other.y - y*other.x,
			w*other.w - x*other.x - y*other.y - z*other.z
		);
	}

	static Quaternion AngleAxis(float angle, const Vector3& axis) {
		float halfAngle = angle * 0.5f * Mathf.PI / 180.0f;
		float sin = std::sin(halfAngle);
		float cos = std::cos(halfAngle);
		Vector3 norm_axis = Vector3::Normalize(axis);
		return Quaternion(
			norm_axis.x * sin,
			norm_axis.y * sin,
			norm_axis.z * sin,
			cos
		);
	};
};
