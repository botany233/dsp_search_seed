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

	// Unity native FromToRotation(up, direction): normalize, build a matrix,
	// convert it to a quaternion, then normalize the quaternion (all float).
	static Quaternion FromUpRotation(Vector3 direction) {
		const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		if(length < 1e-5f) return Quaternion();
		direction = direction / length;
		const float e = (0.0f * direction.x + direction.y * 1.0f) + 0.0f * direction.z;
		if(static_cast<double>(e) > 0.9999990000000025) return Quaternion();
		if(static_cast<double>(e) < -0.9999990000000025) return Quaternion(1.0f,-0.0f,0.0f,0.0f);
		const float vx = direction.z * 1.0f - direction.y * 0.0f,vy = direction.x * 0.0f - 0.0f * direction.z,vz = 0.0f * direction.y - direction.x * 1.0f;
		const float h = (1.0f - e) / (vx * vx + vy * vy + vz * vz);
		const float hx = vx * h,hy = vy * h,hz = vz * h;
		const float m[9] = {
			vx * hx + e,vy * hx + vz,vz * hx - vy,
			vy * hx - vz,vy * hy + e,vz * hy + vx,
			vz * hx + vy,vz * hy - vx,vz * hz + e
		};
		float q[4] = {};
		const float trace = (m[0] + m[4]) + m[8];
		if(trace > 0.0f) {
			const float root = std::sqrt(trace + 1.0f);
			const float scale = 0.5f / root;
			q[0] = (m[5] - m[7]) * scale;
			q[1] = (m[6] - m[2]) * scale;
			q[2] = (m[1] - m[3]) * scale;
			q[3] = root * 0.5f;
		} else {
			int i = m[4] > m[0] ? 1 : 0;
			if(m[8] > m[i * 4]) i = 2;
			const int j = (i + 1) % 3,k = (j + 1) % 3;
			const float root = std::sqrt(((m[i * 4] - m[j * 4]) - m[k * 4]) + 1.0f);
			const float scale = 0.5f / root;
			q[i] = root * 0.5f;
			q[3] = (m[j * 3 + k] - m[k * 3 + j]) * scale;
			q[j] = (m[i * 3 + j] + m[j * 3 + i]) * scale;
			q[k] = (m[i * 3 + k] + m[k * 3 + i]) * scale;
		}
		const float norm = std::sqrt(((q[0] * q[0] + q[1] * q[1]) + q[2] * q[2]) + q[3] * q[3]);
		return Quaternion(q[0] / norm,q[1] / norm,q[2] / norm,q[3] / norm);
	}

	// Managed Unity Quaternion * Vector3: float locals, double expression intermediates.
	Vector3 operator*(const Vector3& point) const {
		const float x2 = x * 2.0f,y2 = y * 2.0f,z2 = z * 2.0f;
		const float xx = x * x2,yy = y * y2,zz = z * z2;
		const float xy = x * y2,xz = x * z2,yz = y * z2;
		const float wx = w * x2,wy = w * y2,wz = w * z2;
		return Vector3(
			static_cast<float>(((1.0 - (static_cast<double>(yy) + zz)) * point.x
				+ (static_cast<double>(xy) - wz) * point.y) + (static_cast<double>(xz) + wy) * point.z),
			static_cast<float>(((static_cast<double>(xy) + wz) * point.x
				+ (1.0 - (static_cast<double>(xx) + zz)) * point.y) + (static_cast<double>(yz) - wx) * point.z),
			static_cast<float>(((static_cast<double>(xz) - wy) * point.x
				+ (static_cast<double>(yz) + wx) * point.y) + (1.0 - (static_cast<double>(xx) + yy)) * point.z)
		);
	}

	Quaternion operator*(const Quaternion& other) const {
		return Quaternion(
			static_cast<float>(((static_cast<double>(w)*other.x + static_cast<double>(x)*other.w) + static_cast<double>(y)*other.z) - static_cast<double>(z)*other.y),
			static_cast<float>(((static_cast<double>(w)*other.y + static_cast<double>(y)*other.w) + static_cast<double>(z)*other.x) - static_cast<double>(x)*other.z),
			static_cast<float>(((static_cast<double>(w)*other.z + static_cast<double>(z)*other.w) + static_cast<double>(x)*other.y) - static_cast<double>(y)*other.x),
			static_cast<float>(((static_cast<double>(w)*other.w - static_cast<double>(x)*other.x) - static_cast<double>(y)*other.y) - static_cast<double>(z)*other.z)
		);
	}

	static Quaternion AngleAxis(float angle,const Vector3& axis);
};
