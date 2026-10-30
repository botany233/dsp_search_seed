// Original 2022 Copyright https://github.com/crazyyao0.
#pragma once

#include <cmath>

class Vector2 {
public:
	float x, y;
	static Vector2 zero() {
		return Vector2{0.0f,0.0f};
	};

	Vector2 operator-() const {
		return Vector2{-x,-y};
	}

	Vector2 operator+(const Vector2& other) const {
		return Vector2{x + other.x,y + other.y};
	}

	Vector2 operator-(const Vector2& other) const {
		return Vector2{x - other.x,y - other.y};
	}

	Vector2 operator*(float s) const
	{
		return Vector2{x * s,y * s};
	}

	Vector2& operator+=(const Vector2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	float sqrMagnitude() {
		return static_cast<float>(static_cast<double>(x) * x + static_cast<double>(y) * y);
	};

	float magnitude() const {
		const double dx = x;
		const double dy = y;
		return static_cast<float>(std::sqrt(dx * dx + dy * dy));
	};

	void Normalize() {
		float num = magnitude();
		if(num > 1e-05f) {
			x /= num;
			y /= num;
		} else {
			x = 0.0f;
			y = 0.0f;
		}
	}

	static Vector2 Normalize(const Vector2& value) {
		float num = value.magnitude();
		if(num > 1e-05f) {
			return Vector2{value.x / num,value.y / num};
		}
		return Vector2{0.0f,0.0f};
	}
};
