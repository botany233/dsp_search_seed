#include "Vector3.hpp"
#include "util.hpp"

// 赋值运算符
Vector3& Vector3::operator=(const Vector3& other) {
	if(this != &other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	return *this;
}

// 运算符重载
Vector3 Vector3::operator+(const Vector3& other) const {
	return Vector3(x + other.x,y + other.y,z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const {
	return Vector3(x - other.x,y - other.y,z - other.z);
}

Vector3 Vector3::operator*(float scalar) const {
	return Vector3(x * scalar,y * scalar,z * scalar);
}

Vector3 Vector3::operator/(float scalar) const {
	if(scalar != 0.0f) {
		return Vector3(x / scalar,y / scalar,z / scalar);
	} else {
		return Vector3(0.0f,0.0f,0.0f);
	}
}

Vector3& Vector3::operator+=(const Vector3& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

Vector3& Vector3::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

Vector3& Vector3::operator/=(float scalar) {
	if(scalar != 0.0f) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
	}
	return *this;
}

bool Vector3::operator==(const Vector3& other) const {
	const float epsilon = 1e-6f;
	return (std::abs(x - other.x) < epsilon) &&
		(std::abs(y - other.y) < epsilon) &&
		(std::abs(z - other.z) < epsilon);
}

bool Vector3::operator!=(const Vector3& other) const {
	return !(*this == other);
}
