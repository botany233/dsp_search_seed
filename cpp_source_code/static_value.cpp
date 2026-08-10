#include <vector>
#include <mutex>
#include <cstdint>

#include "LDB.hpp"
LDB_CLASS LDB;

#include "NameGen.hpp"
NameGen_t NameGen;
const std::string vformat(const char* const zcFormat,...) {
	va_list vaArgs;
	va_start(vaArgs,zcFormat);
	va_list vaArgsCopy;
	va_copy(vaArgsCopy,vaArgs);
	const int iLen = std::vsnprintf(NULL,0,zcFormat,vaArgsCopy);
	va_end(vaArgsCopy);
	std::vector<char> zc(iLen + 1);
	std::vsnprintf(zc.data(),zc.size(),zcFormat,vaArgs);
	va_end(vaArgs);
	return std::string(zc.data(),iLen);
}

std::string ReplaceString(std::string subject,const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while((pos = subject.find(search,pos)) != std::string::npos) {
		subject.replace(pos,search.length(),replace);
		pos += replace.length();
	}
	return subject;
}

#include "PlanetAlgorithm.hpp"
Vector3 PlanetAlgorithm::vertices[VERTICES_DATALENGTH];
int PlanetAlgorithm::indexMap[INDEXMAP_DATALENGTH];
int PlanetAlgorithm::landIndex[LAND_DATALENGTH];

#include <CL/opencl.hpp>
#include "PlanetAlgorithm.hpp"

bool OpenCLManager::SUPPORT_GPU;
int OpenCLManager::local_size;
int OpenCLManager::device_id;
std::vector<cl::Device> OpenCLManager::devices;
std::vector<string> OpenCLManager::devices_info;
cl::Context OpenCLManager::context;
cl::Device OpenCLManager::device;
cl::Program OpenCLManager::program;
cl::Buffer OpenCLManager::vertices_buffer;
size_t OpenCLManager::cfg_version;
mutex OpenCLManager::lock;
int OpenCLManager::max_worker = 8;
int OpenCLManager::cur_worker = 0;

#include "Vector3.hpp"

constexpr double PiOver4 = 0.7853981633974483;
constexpr double SmallCut = 0.0078125;
constexpr double TinyCut = 0.0001220703125;
constexpr double TwoOverPi = 0.6366197723675814;
constexpr double PiOver2Hi = 1.5707963267341256;
constexpr double PiOver2Lo = 6.077100506506192e-11;

constexpr double SinC9 = 2.7557319223985893e-06;
constexpr double SinC7 = -0.0001984126984126984;
constexpr double SinC5 = 0.008333333333333333;
constexpr double SinC3 = -0.16666666666666666;

constexpr double CosC8 = -2.755731922398589e-07;
constexpr double CosC6 = 2.4801587301587298e-05;
constexpr double CosC4 = -0.0013888888888888887;
constexpr double CosC2 = 0.041666666666666664;

constexpr double FastReduceLimit = 16779436.0;

static double absBits(double value) {
	uint64_t bits = std::bit_cast<std::uint64_t>(value);
	bits &= 0x7FFFFFFFFFFFFFFFull;
	return std::bit_cast<double>(bits);
}

static double sinPoly(double x) {
	const auto xx = x * x;
	auto p = std::fma(xx,SinC9,SinC7);
	p = std::fma(p,xx,SinC5);
	p = std::fma(p,xx,SinC3);
	return std::fma(p,x * xx,x);
}

static int reduce(double magnitude,double& reduced) {
	const auto scaled = std::fma(TwoOverPi,magnitude,0.5);
	const auto n = static_cast<double>(static_cast<int>(scaled));
	const auto high = std::fma(-n,PiOver2Hi,magnitude);
	reduced = high - n * PiOver2Lo;
	return static_cast<int>(scaled) & 3;
}

static double cosPolyReduced(double x) {
	const auto xx = x * x;
	const auto base = std::fma(xx,-0.5,1.0);
	auto p = std::fma(xx,CosC8,CosC6);
	p = std::fma(p,xx,CosC4);
	p = std::fma(p,xx,CosC2);
	return std::fma(p,xx * xx,base);
}

static bool negative(double value) {
	std::uint64_t bits = std::bit_cast<uint64_t>(value);
	return (bits >> 63) != 0;
}

static float ucrtSinf(float value) {
	const auto x = static_cast<double>(value);
	const auto magnitude = absBits(x);
	if(magnitude <= PiOver4) {
		if(magnitude >= SmallCut) return static_cast<float>(sinPoly(x));
		if(magnitude < TinyCut) return value;
		return static_cast<float>(std::fma(-(x * 0.5),x * x,x));
	}
	if(magnitude >= FastReduceLimit) return static_cast<float>(std::sin(x));

	double reduced;
	const auto region = reduce(magnitude,reduced);
	auto result = ((region & 1) != 0) ? cosPolyReduced(reduced) : sinPoly(reduced);
	if(((region == 2 || region == 3) ? 1 : 0) ^ (negative(x) ? 1 : 0))
		result = -result;
	return static_cast<float>(result);
}

static double cosPoly(double x) {
	const auto xx = x * x;
	const auto base = 1.0 - xx * 0.5;
	auto p = std::fma(xx,CosC8,CosC6);
	p = std::fma(p,xx,CosC4);
	p = std::fma(p,xx,CosC2);
	return std::fma(p,xx * xx,base);
}

static float ucrtCosf(float value) {
	const auto x = static_cast<double>(value);
	const auto magnitude = absBits(x);
	if(magnitude <= PiOver4) {
		if(magnitude >= SmallCut) return static_cast<float>(cosPoly(x));
		if(magnitude < TinyCut) return 1.0f;
		return static_cast<float>(std::fma(-(x * 0.5),x,1.0));
	}
	if(magnitude >= FastReduceLimit) return static_cast<float>(std::cos(x));

	double reduced;
	const auto region = reduce(magnitude,reduced);
	auto result = ((region & 1) != 0) ? sinPoly(reduced) : cosPolyReduced(reduced);
	if(region == 1 || region == 2) result = -result;
	return static_cast<float>(result);
}

static Vector3 nativeSlerpOrthonormal(const Vector3& value) {
	const auto threshold = std::bit_cast<float>(0x3f3504f3u);
	const auto absZ = std::abs(value.z);
	if(absZ <= threshold) {
		const auto xSquared = value.x * value.x;
		const auto ySquared = value.y * value.y;
		const auto lengthSquared = xSquared + ySquared;
		const auto length = std::sqrt(lengthSquared);
		const auto inverse = 1.0f / length;
		return {(-value.y) * inverse,value.x * inverse,0.0f};
	}
	const auto zSquared = value.z * value.z;
	const auto ySquared = value.y * value.y;
	const auto lengthSquared = zSquared + ySquared;
	const auto length = std::sqrt(lengthSquared);
	const auto inverse = 1.0f / length;
	return {0.0f,(-value.z) * inverse,value.y * inverse};
};

static Vector3 nativeRotateAroundAxis(const Vector3& value,const Vector3& axis,float angle,float magnitude) {
	const auto sine = ucrtSinf(angle);
	const auto cosine = ucrtCosf(angle);
	const auto oneMinusCosine = 1.0f - cosine;

	const auto x = axis.x;
	const auto y = axis.y;
	const auto z = axis.z;
	const auto xy = y * x;
	const auto xz = z * x;
	const auto yz = z * y;
	const auto xs = x * sine;
	const auto ys = y * sine;
	const auto zs = z * sine;

	float m0 = (x * x) * oneMinusCosine;
	m0 = m0 + cosine;
	const auto qxy = oneMinusCosine * xy;
	const auto m3 = qxy - zs;
	const auto m1 = qxy + zs;
	const auto m6 = oneMinusCosine * xz + ys;
	float m4 = (y * y) * oneMinusCosine;
	m4 = m4 + cosine;
	float m8 = (z * z) * oneMinusCosine;
	m8 = m8 + cosine;
	const auto m2 = oneMinusCosine * xz - ys;
	const auto m7 = oneMinusCosine * yz - xs;
	const auto m5 = oneMinusCosine * yz + xs;

	auto rx = m3 * value.y;
	rx = rx + m0 * value.x;
	rx = rx + m6 * value.z;
	auto ry = m4 * value.y;
	ry = ry + m1 * value.x;
	ry = ry + m7 * value.z;
	auto rz = m5 * value.y;
	rz = rz + m2 * value.x;
	rz = rz + m8 * value.z;
	return {rx * magnitude,ry * magnitude,rz * magnitude};
}

static float ucrtAcosf(float value) {
	const auto raw = [&] {
		std::uint32_t bits;
		std::memcpy(&bits,&value,sizeof bits);
		return bits;
	}();
	const auto exponent = (raw >> 23) & 0xffu;
	const auto magnitude = std::bit_cast<float>(raw & 0x7fffffffu);
	if(exponent < 0x65u)
		return std::bit_cast<float>(0x3fc90fdbu);
	if(exponent >= 0x7fu) {
		if(value == 1.0f)
			return 0.0f;
		if(value == -1.0f)
			return std::bit_cast<float>(0x40490fdbu);
		return std::numeric_limits<float>::quiet_NaN();
	}

	float z;
	float root = 0.0f;
	if(exponent < 0x7eu) {
		z = magnitude * magnitude;
	} else {
		auto oneMinusMagnitude = 1.0f - magnitude;
		z = oneMinusMagnitude * 0.5f;
		root = std::sqrt(z);
	}

	const auto p0 = std::bit_cast<float>(0xbc5b3fe1u);
	const auto p1 = std::bit_cast<float>(0x3b81ce6bu);
	const auto p2 = std::bit_cast<float>(0x3d678bddu);
	const auto p3 = std::bit_cast<float>(0x3e3c94dcu);
	const auto d0 = std::bit_cast<float>(0x3f8d6fa5u);
	const auto d1 = std::bit_cast<float>(0x3f561f0du);
	auto numerator = p0 - z * p1;
	numerator = numerator * z;
	numerator = numerator - p2;
	numerator = numerator * z;
	numerator = numerator + p3;
	numerator = numerator * z;
	const auto denominator = d0 - z * d1;
	const auto ratio = numerator / denominator;

	constexpr double piOver2Low = 6.123233995736766e-17;
	constexpr double piOver2 = 1.5707963267948966;
	constexpr double pi = 3.141592653589793;
	if(exponent < 0x7eu) {
		const auto product = ratio * value;
		return static_cast<float>(piOver2 -
			(static_cast<double>(value) -
			(piOver2Low - static_cast<double>(product))));
	}
	if(value >= 0.0f) {
		const auto high = std::bit_cast<float>([](std::uint32_t bits) {
			return bits & 0xffff0000u;
		}([&] {
			std::uint32_t bits;
			std::memcpy(&bits,&root,sizeof bits);
			return bits;
		}()));
		auto twiceRoot = root + root;
		const auto correction = (z - high * high) / (high + root);
		twiceRoot = twiceRoot * ratio;
		auto twiceCorrection = correction + correction;
		auto result = twiceRoot + twiceCorrection;
		result = result + high * 2.0f;
		return result;
	}
	const auto product = root * ratio;
	const auto inner = static_cast<double>(root) +
		(static_cast<double>(product) - piOver2Low);
	return static_cast<float>(pi - (inner + inner));
}

Vector3 Vector3::Slerp(const Vector3& a,const Vector3& b,float t) {
	if(0.0f > t) t = 0.0f;
	else t = std::min(1.0f,t);

	const auto axSquared = a.x * a.x;
	const auto aySquared = a.y * a.y;
	const auto azSquared = a.z * a.z;
	const auto lengthSquaredA0 = axSquared + aySquared;
	const auto lengthSquaredA = lengthSquaredA0 + azSquared;
	const auto lengthA = std::sqrt(lengthSquaredA);

	const auto bxSquared = b.x * b.x;
	const auto bySquared = b.y * b.y;
	const auto bzSquared = b.z * b.z;
	const auto lengthSquaredB0 = bxSquared + bySquared;
	const auto lengthSquaredB = lengthSquaredB0 + bzSquared;
	const auto lengthB = std::sqrt(lengthSquaredB);

	const auto inverse = 1.0f - t;
	if(lengthA < 1e-5f || lengthB < 1e-5f)
		return nativeLerp(a,b,t);

	const auto magnitudeA = inverse * lengthA;
	const auto magnitudeB = lengthB * t;
	const auto magnitude = magnitudeA + magnitudeB;

	const auto dotX = b.x * a.x;
	const auto dotY = b.y * a.y;
	const auto dotXY = dotX + dotY;
	const auto dotZ = b.z * a.z;
	const auto numerator = dotXY + dotZ;
	const auto denominator = lengthB * lengthA;
	const auto cosine = numerator / denominator;
	const auto threshold = std::bit_cast<float>(0x3f7fff58u);
	if(cosine > threshold)
		return nativeLerp(a,b,t);

	const Vector3 normalA{a.x / lengthA,a.y / lengthA,a.z / lengthA};
	if(cosine < -threshold) {
		const auto axis = nativeSlerpOrthonormal(normalA);
		const auto angle = t * Mathf.PI;
		return nativeRotateAroundAxis(normalA,axis,angle,magnitude);
	}

	const auto crossX = b.z * a.y - b.y * a.z;
	const auto crossY = b.x * a.z - b.z * a.x;
	const auto crossZ = b.y * a.x - b.x * a.y;
	const auto crossYSquared = crossY * crossY;
	const auto crossXSquared = crossX * crossX;
	const auto crossLengthSquared0 = crossYSquared + crossXSquared;
	const auto crossZSquared = crossZ * crossZ;
	const auto crossLengthSquared = crossLengthSquared0 + crossZSquared;
	const auto crossLength = std::sqrt(crossLengthSquared);
	const Vector3 axis{crossX / crossLength,crossY / crossLength,
		crossZ / crossLength};
	const auto angle = ucrtAcosf(cosine) * t;
	return nativeRotateAroundAxis(normalA,axis,angle,magnitude);
}
