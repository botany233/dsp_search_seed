#pragma once
#include <tuple>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <mutex>
#include <glm/glm.hpp>
#include <CL/opencl.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "astro_class.hpp"
#include "DSPGen.hpp"
#include "util.hpp"
#include "Maths.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "DotNet35Random.hpp"
#include "const_value.hpp"
#include "SimplexNoise.hpp"
#include "RandomTable.hpp"
#include "LDB.hpp"
#include "defines.hpp"

using namespace std;

#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4838)

class OpenCLManager
{
public:
	static bool SUPPORT_GPU;
	static int local_size;
	static int device_id;
	static std::vector<cl::Device> devices;
	static std::vector<std::string> devices_info;
	static cl::Context context;
	static cl::Device device;
	static cl::Program program;
	static cl::Buffer vertices_buffer;
	static size_t cfg_version;
	static mutex lock;
	static int max_worker;
	static int cur_worker;

	static void do_init() {
		static bool is_init = false;
		if(is_init)
			return;
		is_init = true;
		cfg_version = 0;
		set_device_id(-1);
		set_local_size();
		max_worker = 4;
	}

	static bool set_device_id(int input_device_id);
	
	static int get_device_id() {
		lock_guard<mutex> lck(lock);
		if(SUPPORT_GPU)
			return device_id;
		else
			return -1;
	}

	static void set_local_size(int size = 32) {
		lock_guard<mutex> lck(lock);
		local_size = max(size,32);
	}

	static int get_local_size() {
		lock_guard<mutex> lck(lock);
		return local_size;
	}

	static std::vector<std::string> get_devices_info() {
		lock_guard<mutex> lck(lock);
		return devices_info;
	}

	static void AddSources(cl::Program::Sources& sources,const string& file_name) {
		ifstream file(file_name);
		string* source_code = new string(istreambuf_iterator<char>(file),(istreambuf_iterator<char>()));
		sources.push_back((*source_code).c_str());
	}

	static void set_max_worker(int num) {
		lock_guard<mutex> lck(lock);
		max_worker = num;
	}

	static int get_max_worker() {
		lock_guard<mutex> lck(lock);
		return max_worker;
	}

	static bool get_worker() {
		lock_guard<mutex> lck(lock);
		if(!SUPPORT_GPU)
			return false;
		if(cur_worker>=max_worker)
			return false;
		cur_worker++;
		return true;
	}

	static void return_worker() {
		lock_guard<mutex> lck(lock);
		cur_worker--;
	}
};

class ThreadLocalBuffers {
public:
	static constexpr size_t PERM_BUF_SIZE = sizeof(int) * PERM_LENGTH;
	static constexpr size_t DOUBLE_BUF_SIZE = sizeof(double) * 80;
	static constexpr size_t FLOAT_BUF_SIZE = sizeof(float) * 320;
	
	static constexpr size_t ALIGN = alignof(std::max_align_t);

	static constexpr size_t OFF_DOUBLE = PERM_BUF_SIZE * 8;
	static constexpr size_t OFF_FLOAT = OFF_DOUBLE + DOUBLE_BUF_SIZE;
	static constexpr size_t CACHE_SIZE = OFF_FLOAT + FLOAT_BUF_SIZE;

	static constexpr size_t RESULT_SIZE = sizeof(unsigned short) * LAND_DATALENGTH;

	alignas(std::max_align_t) std::byte cache[CACHE_SIZE];

	int* perm_buffer_1() { return reinterpret_cast<int*>(cache); }
	int* perm_buffer_2() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE * 2); }
	int* perm_buffer_3() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE * 4); }
	int* perm_buffer_4() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE * 6); }
	int* permMod12_buffer_1() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE); }
	int* permMod12_buffer_2() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE * 3); }
	int* permMod12_buffer_3() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE * 5); }
	int* permMod12_buffer_4() { return reinterpret_cast<int*>(cache + PERM_BUF_SIZE * 7); }

	double* double_buffer() { return reinterpret_cast<double*>(cache + OFF_DOUBLE); }
	float* float_buffer() { return reinterpret_cast<float*>(cache + OFF_FLOAT); }

	cl::CommandQueue queue;
	cl::Buffer buffer;
	cl::Buffer heightData_buffer;
	//cl::Buffer debugData_buffer;

	size_t local_cfg_version = 0;
	
	void check_init() {
		lock_guard<mutex> lck(OpenCLManager::lock);
		if(local_cfg_version != OpenCLManager::cfg_version) {
			queue = cl::CommandQueue(OpenCLManager::context,OpenCLManager::device);
			buffer = cl::Buffer(OpenCLManager::context,CL_MEM_READ_ONLY,CACHE_SIZE);
			heightData_buffer = cl::Buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,RESULT_SIZE);
			//cl::Buffer debugData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(float) * DATALENGTH);
			local_cfg_version = OpenCLManager::cfg_version;
		}
	}

	void upload_buffer() {
		queue.enqueueWriteBuffer(buffer,CL_FALSE,0,CACHE_SIZE,cache);
	}

	void download_buffer(unsigned short *dst) {
		queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,RESULT_SIZE,dst);
	}
};

inline ThreadLocalBuffers& get_tls_buffers() {
	static thread_local ThreadLocalBuffers tls;
	tls.check_init();
	return tls;
}

static GalaxyClassSimple galaxy_to_simple(const GalaxyClass& galaxy) {
	GalaxyClassSimple galaxy_simple;
	galaxy_simple.resource_multiplier = galaxy.resource_multiplier;
	galaxy_simple.is_infinite_resource = galaxy.is_infinite_resource;
	galaxy_simple.is_rare_resource = galaxy.is_rare_resource;
	galaxy_simple.birthPlanetId = galaxy.birthPlanetId;
	return galaxy_simple;
}

static StarClassSimple star_to_simple(const StarClass& star) {
	StarClassSimple star_simple;
	star_simple.type = star.type;
	star_simple.mass = star.mass;
	star_simple.index = star.index;
	star_simple.spectr = star.spectr;
	star_simple.uPosition = star.uPosition;
	star_simple.resourceCoef = star.resourceCoef;
	return star_simple;
}

static PlanetClassSimple planet_to_simple(const PlanetClass& planet) {
	PlanetClassSimple planet_simple;
	planet_simple.id = planet.id;
	planet_simple.seed = planet.seed;
	planet_simple.info_seed = planet.infoSeed;
	planet_simple.type = planet.type;
	planet_simple.theme = planet.theme;
	planet_simple.mod_x = planet.mod_x;
	planet_simple.mod_y = planet.mod_y;
	planet_simple.algoId = planet.algoId;
	planet_simple.waterItemId = planet.waterItemId;
	planet_simple.orbitRadius = planet.orbitRadius;
	planet_simple.orbitAround = planet.orbitAround;
	planet_simple.orbitIndex = planet.orbitIndex;
	return planet_simple;
}

class PlanetAlgorithm
{
protected:
	static int trans(float x,int pr) {
		int num = (int)((Mathf.Sqrt(x + 0.23f) - 0.4795832f) / 0.6294705f * (float)pr);
		if(num >= pr)
			num = pr - 1;
		return num;
	}

	static int PositionHash(Vector3 v,int corner = 0) {
		if(corner == 0)
			corner = ((v.x > 0.0f) ? 1 : 0) + ((v.y > 0.0f) ? 2 : 0) + ((v.z > 0.0f) ? 4 : 0);
		if(v.x < 0.0f)
			v.x = 0.0f - v.x;
		if(v.y < 0.0f)
			v.y = 0.0f - v.y;
		if(v.z < 0.0f)
			v.z = 0.0f - v.z;
		if((double)v.x < 1E-06 && (double)v.y < 1E-06 && (double)v.z < 1E-06)
			return 0;
		int num = 0;
		int num2 = 0;
		int num3 = 0;
		if(v.x >= v.y && v.x >= v.z) {
			num = 0;
			num2 = trans(v.z / v.x,INDEXMAP_PRECISION);
			num3 = trans(v.y / v.x,INDEXMAP_PRECISION);
		} else if(v.y >= v.x && v.y >= v.z) {
			num = 1;
			num2 = trans(v.x / v.y,INDEXMAP_PRECISION);
			num3 = trans(v.z / v.y,INDEXMAP_PRECISION);
		} else {
			num = 2;
			num2 = trans(v.x / v.z,INDEXMAP_PRECISION);
			num3 = trans(v.y / v.z,INDEXMAP_PRECISION);
		}
		return num2 + num3 * INDEXMAP_PRECISION + num * INDEXMAP_FACE_STRIDE + corner * INDEXMAP_CORNER_STRIDE;
	};

	static void CalcVerts() {
		static bool is_init = false;
		if(is_init)
			return;
		is_init = true;
		int num = (PRECISION + 1) * 2;
		int num2 = PRECISION + 1;
		Vector3 poles[] = {
			Vector3::right(),
			Vector3::left(),
			Vector3::up(),
			Vector3::down(),
			Vector3::forward(),
			Vector3::back()
		};
		for(int i=0;i<INDEXMAP_DATALENGTH;i++)
			indexMap[i] = -1;
		for(int j = 0; j < VERTICES_DATALENGTH; j++) {
			int num3 = j % num;
			int num4 = j / num;
			int num5 = num3 % num2;
			int num6 = num4 % num2;
			int num7 = (((num3 >= num2) ? 1 : 0) + ((num4 >= num2) ? 1 : 0) * 2) * 2 + ((num5 < num6) ? 1 : 0);
			float num8 = ((num5 >= num6) ? (PRECISION - num5) : num5);
			float num9 = ((num5 >= num6) ? num6 : (PRECISION - num6));
			float num10 = (float)PRECISION - num9;
			num9 /= (float)PRECISION;
			num8 = ((num10 > 0.0f) ? (num8 / num10) : 0.0f);
			int num11 = 0;
			Vector3 a;
			Vector3 a2;
			Vector3 b;
			switch(num7)
			{
			case 0:
			a = poles[2];
			a2 = poles[0];
			b = poles[4];
			num11 = 7;
			break;
			case 1:
			a = poles[3];
			a2 = poles[4];
			b = poles[0];
			num11 = 5;
			break;
			case 2:
			a = poles[2];
			a2 = poles[4];
			b = poles[1];
			num11 = 6;
			break;
			case 3:
			a = poles[3];
			a2 = poles[1];
			b = poles[4];
			num11 = 4;
			break;
			case 4:
			a = poles[2];
			a2 = poles[1];
			b = poles[5];
			num11 = 2;
			break;
			case 5:
			a = poles[3];
			a2 = poles[5];
			b = poles[1];
			num11 = 0;
			break;
			case 6:
			a = poles[2];
			a2 = poles[5];
			b = poles[0];
			num11 = 3;
			break;
			case 7:
			a = poles[3];
			a2 = poles[0];
			b = poles[5];
			num11 = 1;
			break;
			default:
			a = poles[2];
			a2 = poles[0];
			b = poles[4];
			num11 = 7;
			break;
			}
			vertices[j] = Vector3::Slerp(Vector3::Slerp(a,b,num9),Vector3::Slerp(a2,b,num9),num8);
			int num12 = PositionHash(vertices[j],num11);
			if(indexMap[num12] == -1)
				indexMap[num12] = j;
		}
		for(int k = 1; k < INDEXMAP_DATALENGTH; k++) {
			if(indexMap[k] == -1)
				indexMap[k] = indexMap[k - 1];
		}
	};

	//unsigned short GetHeight(int index) {
	//	if(heightData[index] == 0)
	//		GenerateHeight1(index);
	//	return heightData[index];
	//}

public:
	static Vector3 vertices[VERTICES_DATALENGTH];
	static int indexMap[INDEXMAP_DATALENGTH];
	static int landIndex[LAND_DATALENGTH];
	vector<unsigned short> heightData;
	//vector<float> debugData;

	virtual ~PlanetAlgorithm() = default;

	static void do_init() {
		static bool is_init = false;
		if(is_init)
			return;
		is_init = true;
		CalcVerts();
		int index = 0;
		for(int i = 0; i < VERTICES_DATALENGTH; i++) {
			int num5 = i % STRIDE;
			int num6 = i / STRIDE;
			if(num5 > LANDPERCENT_NUM)
				num5--;
			if(num6 > LANDPERCENT_NUM)
				num6--;
			if(num5 & num6 & 1)
				landIndex[index++] = i;
		}
	}
	
	glm::vec3 vector3_to_glm(const Vector3& vec) {
		return glm::vec3(vec.x,vec.y,vec.z);
	}

	Vector3 glm_to_vector3(const glm::vec3& vec) {
		return Vector3(vec.x,vec.y,vec.z);
	}

	// Temporary birth state; parent motion needs only its orbit, not its spin.
	struct BirthMotion {
		double orbitalPeriod = 0.0;
		Quaternion orbitRotation;
		Pose pose;
	};
	
	static BirthMotion PredictBirthMotion(const PlanetClassSimple& planet,const double time,bool includeRotation) {
		const StarClassSimple& star = *planet.star;
		DotNet35Random random(planet.info_seed);
		random.NextDouble(); // num3: orbit radius is already available
		random.NextDouble(); // num4
		double num5 = random.NextDouble();
		double num6 = random.NextDouble();
		double num7 = random.NextDouble();

		BirthMotion parent;
		if(planet.orbitAroundPlanet != nullptr)
			parent = PredictBirthMotion(*planet.orbitAroundPlanet,time,false);

		BirthMotion motion;
		float inclination = (float)(num5 * 16.0 - 8.0);
		if(planet.orbitAround > 0)
			inclination *= 2.2f;
		if(star.type >= EStarType::NeutronStar)
		{
			if((double)inclination > 0.0)
				inclination += 3.0f;
			else
				inclination -= 3.0f;
		}
		float f1 = planet.orbitRadius;
		motion.orbitalPeriod = planet.orbitAroundPlanet != nullptr ? Math.Sqrt(39.4784176043574 * (double)f1 * (double)f1 * (double)f1 / 1.08308421068537E-08) : Math.Sqrt(39.4784176043574 * (double)f1 * (double)f1 * (double)f1 / (1.35385519905204E-06 * (double)star.mass));
		float orbitPhase = (float)(num7 * 360.0);
		motion.orbitRotation = Quaternion::AngleAxis((float)(num6 * 360.0),Vector3::up()) * Quaternion::AngleAxis(inclination,Vector3::forward());
		if(planet.orbitAroundPlanet != nullptr)
			motion.orbitRotation = parent.orbitRotation * motion.orbitRotation;

		double num = time / motion.orbitalPeriod + (double)orbitPhase / 360.0;
		int num2 = (int)(num + 0.1);
		num -= (double)num2;
		num *= Math.PI * 2.0;
		motion.pose.position = Maths::QRotate(motion.orbitRotation,Vector3((float)Math.Cos(num) * planet.orbitRadius,0.0f,(float)Math.Sin(num) * planet.orbitRadius));
		if(planet.orbitAroundPlanet != nullptr)
		{
			motion.pose.position.x += parent.pose.position.x;
			motion.pose.position.y += parent.pose.position.y;
			motion.pose.position.z += parent.pose.position.z;
		}
		if(!includeRotation)
			return motion;

		double num8 = random.NextDouble();
		double num9 = random.NextDouble();
		double num10 = random.NextDouble();
		double num11 = random.NextDouble();
		double num12 = random.NextDouble();
		random.NextDouble(); // num13
		random.NextDouble(); // num14
		random.NextDouble(); // rand1
		double num15 = random.NextDouble();
		float obliquity;
		if(num15 < 0.0399999991059303)
		{
			obliquity = (float)(num8 * (num9 - 0.5) * 39.9);
			if((double)obliquity < 0.0)
				obliquity -= 70.0f;
			else
				obliquity += 70.0f;
		} else if(num15 < 0.100000001490116)
		{
			obliquity = (float)(num8 * (num9 - 0.5) * 80.0);
			if((double)obliquity < 0.0)
				obliquity -= 30.0f;
			else
				obliquity += 30.0f;
		} else
			obliquity = (float)(num8 * (num9 - 0.5) * 60.0);
		bool gasGiant = planet.type == EPlanetType::Gas;
		double rotationPeriod = (num10 * num11 * 1000.0 + 400.0) * (planet.orbitAround == 0 ? (double)Mathf.Pow(f1,0.25f) : 1.0) * (gasGiant ? 0.200000002980232 : 1.0);
		if(!gasGiant)
		{
			if(star.type == EStarType::WhiteDwarf)
				rotationPeriod *= 0.5;
			else if(star.type == EStarType::NeutronStar)
				rotationPeriod *= 0.200000002980232;
			else if(star.type == EStarType::BlackHole)
				rotationPeriod *= 0.150000005960464;
		}
		float rotationPhase = (float)(num12 * 360.0);
		double num17 = planet.orbitAround == 0 ? motion.orbitalPeriod : parent.orbitalPeriod;
		rotationPeriod = 1.0 / (1.0 / num17 + 1.0 / rotationPeriod);
		if(planet.orbitAround == 0 && planet.orbitIndex <= 4 && !gasGiant)
		{
			if(num15 > 0.959999978542328)
			{
				obliquity *= 0.01f;
				rotationPeriod = motion.orbitalPeriod;
			} else if(num15 > 0.930000007152557)
			{
				obliquity *= 0.1f;
				rotationPeriod = motion.orbitalPeriod * 0.5;
			} else if(num15 > 0.899999976158142)
			{
				obliquity *= 0.2f;
				rotationPeriod = motion.orbitalPeriod * 0.25;
			}
		}
		if(num15 > 0.85 && num15 <= 0.9)
			rotationPeriod = -rotationPeriod;
		Quaternion systemRotation = motion.orbitRotation * Quaternion::AngleAxis(obliquity,Vector3::forward());
		double num3 = time / rotationPeriod + (double)rotationPhase / 360.0;
		int num4 = (int)(num3 + 0.1);
		num3 = (num3 - (double)num4) * 360.0;
		motion.pose.rotation = systemRotation * Quaternion::AngleAxis((float)num3,Vector3::down());
		return motion;
	}
	
	tuple<Vector3,Vector3,Vector3> GenBirthPoints(const PlanetClassSimple& planet,const int _birthSeed,const VectorLF3& star_uPosition) {
		DotNet35Random dotNet35Random = DotNet35Random(_birthSeed);
		Pose pose = PredictBirthMotion(planet,85.0,true).pose;
		Vector3 vector = Maths::QInvRotateLF(pose.rotation,star_uPosition - pose.position * 40000.0);
		vector.Normalize();
		Vector3 normalized = Vector3::Normalize(Vector3::Cross(vector,Vector3::up()));
		Vector3 normalized2 = Vector3::Normalize(Vector3::Cross(normalized,vector));
		int i = 0;
		int num;
		Vector3 birthPoint,birthResourcePoint0,birthResourcePoint1;
		for(num = 256; i < num; i++)
		{
			float num2 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.5f;
			float num3 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.5f;
			Vector3 vector2 = vector + normalized * num2 + normalized2 * num3;
			vector2.Normalize();
			birthPoint = vector2 * (NORMAL_PLANET_REAL_RADIUS + 0.2f + 1.45f);
			normalized = Vector3::Normalize(Vector3::Cross(vector2,Vector3::up()));
			normalized2 = Vector3::Normalize(Vector3::Cross(normalized,vector2));
			bool flag = false;
			for(int j = 0; j < 10; j++)
			{
				float x = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0);
				float y = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0);
				Vector2 vector3 = Vector2::Normalize(Vector2(x,y)) * 0.1f;
				Vector2 vector4 = -vector3;
				float num4 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.06f;
				float num5 = (float)(dotNet35Random.NextDouble() * 2.0 - 1.0) * 0.06f;
				vector4.x += num4;
				vector4.y += num5;
				Vector3 normalized3 = Vector3::Normalize((vector2 + normalized * vector3.x + normalized2 * vector3.y));
				Vector3 normalized4 = Vector3::Normalize((vector2 + normalized * vector4.x + normalized2 * vector4.y));
				birthResourcePoint0 = Vector3::Normalize(normalized3);
				birthResourcePoint1 = Vector3::Normalize(normalized4);
				float num6 = NORMAL_PLANET_REAL_RADIUS + 0.2f;
				if(QueryHeight(vector2) > num6 && QueryHeight(normalized3) > num6 && QueryHeight(normalized4) > num6)
				{
					Vector3 vpos = normalized3 + normalized * 0.03f;
					Vector3 vpos2 = normalized3 - normalized * 0.03f;
					Vector3 vpos3 = normalized3 + normalized2 * 0.03f;
					Vector3 vpos4 = normalized3 - normalized2 * 0.03f;
					Vector3 vpos5 = normalized4 + normalized * 0.03f;
					Vector3 vpos6 = normalized4 - normalized * 0.03f;
					Vector3 vpos7 = normalized4 + normalized2 * 0.03f;
					Vector3 vpos8 = normalized4 - normalized2 * 0.03f;
					if(QueryHeight(vpos) > num6 && QueryHeight(vpos2) > num6 && QueryHeight(vpos3) > num6 && QueryHeight(vpos4) > num6 && QueryHeight(vpos5) > num6 && QueryHeight(vpos6) > num6 && QueryHeight(vpos7) > num6 && QueryHeight(vpos8) > num6)
					{
						flag = true;
						break;
					}
				}
			}
			if(flag)
			{
				break;
			}
		}
		if(i >= num)
		{
			birthPoint = Vector3(0.0f,NORMAL_PLANET_REAL_RADIUS + 5.0f,0.0f);
		}
		return {birthPoint,birthResourcePoint0,birthResourcePoint1};
	}

	float QueryHeight(Vector3 vpos) {
		vpos.Normalize();
		int num = PositionHash(vpos);
		int num2 = indexMap[num];
		float num3 = Mathf.PI / (float)(PRECISION * 2) * 1.2f;
		float num4 = num3 * num3;
		float num5 = 0.0f;
		float num6 = 0.0f;
		int num8_cache[25] = {0};
		float num9_cache[25] = {0};
		int count = 0;
		for(int i = -1; i <= 3; i++) {
			for(int j = -1; j <= 3; j++) {
				int num8 = num2 + i + j * STRIDE;
				if((unsigned int)num8 < VERTICES_DATALENGTH) {
					float sqrMagnitude = (vertices[num8] - vpos).sqrMagnitude();
					if(sqrMagnitude <= num4) {
						float num9 = 1.0f - Mathf.Sqrt(sqrMagnitude) / num3;
						num8_cache[count] = num8;
						num9_cache[count] = num9;
						count++;
					}
				}
			}
		}
		int index_cache[25] = {0};
		int cur_num = 0;
		for(int i=0;i<count;i++) {
			if(heightData[num8_cache[i]] != 0)
				continue;
			index_cache[cur_num++] = num8_cache[i];
		}
		GenerateHeights(index_cache,cur_num);
		
		for(int i=0;i<count;i++) {
			num5 += num9_cache[i];
			num6 += (int)heightData[num8_cache[i]] * num9_cache[i];
		}
		if(num5 == 0.0f) {
			if(heightData[0]==0)
				GenerateHeight1(0);
			return (float)(int)heightData[0] * 0.01f;
		}
		return num6 / num5 * 0.01f;
	};

	void get_veins(const GalaxyClass& galaxy,const StarClass& star,PlanetClass& planet) {
		PlanetClassSimple planet_simple = planet_to_simple(planet);
		StarClassSimple star_simple = star_to_simple(star);
		GalaxyClassSimple galaxy_simple = galaxy_to_simple(galaxy);
		star_simple.galaxy = &galaxy_simple;
		planet_simple.star = &star_simple;
		PlanetClassSimple planet_simple_orbit;
		if(planet.orbitAroundPlanet != nullptr) {
			planet_simple_orbit = planet_to_simple(*planet.orbitAroundPlanet);
			planet_simple_orbit.star = &star_simple;
			planet_simple.orbitAroundPlanet = &planet_simple_orbit;
		}
		this->GenerateTerrain(planet_simple,true);
		float land_percent = CalcLandPercent(planet_simple);
		this->GenerateVeins(planet_simple,galaxy.birthPlanetId);
		for(int i=0; i < 14; i++) {
			planet.veins_point[i] = planet_simple.veins_point[i];
			planet.veins_amount[i] = planet_simple.veins_amount[i];
		}
		planet.landPercent = land_percent;
	}
	
	float CalcLandPercent(const PlanetClassSimple& planet) {
		if(planet.theme == 16) //水世界
			return 1.0f;
		if(planet.type == EPlanetType::Gas)
			return 0.0f;

		bool need_gen = false;
		for(int i=0;i<LAND_DATALENGTH;i++) {
			if(heightData[landIndex[i]] ==0) {
				need_gen = true;
				break;
			}
		}
		GenerateHeights(landIndex,LAND_DATALENGTH);

		float threshold = NORMAL_PLANET_RADIUS * 100.0f - 20.0f;
		int num3 = 0;
		int num4 = 0;
		for(int i = 0; i < VERTICES_DATALENGTH; i++) {
			int num5 = i % STRIDE;
			int num6 = i / STRIDE;
			if(num5 > LANDPERCENT_NUM)
				num5--;
			if(num6 > LANDPERCENT_NUM)
				num6--;
			if(num5 & num6 & 1) {
				if((float)heightData[i] >= threshold)
					num4++;
				num3++;
			}
		}
		return ((num3 > 0) ? ((float)num4 / (float)num3) : 0.0f);
	}

	void GenerateHeights(int* index,int count) {
		#ifdef SUPPORT_AVX2
		while(count >=4) {
			GenerateHeight4(index);
			count -= 4;
			index += 4;
		}
		if(count >= 2) {
			GenerateHeight2(index);
			count -= 2;
			index += 2;
		}
		if(count > 0)
			GenerateHeight1(*index);
		#else
		for(int i=0;i<count;i++)
			GenerateHeight1(*(index+i));
		#endif
	}

	virtual void GenerateHeight1(int index) = 0;

	virtual void GenerateHeight2(int* index_arr) {
		for(int i=0;i<2;i++) {
			GenerateHeight1(index_arr[i]);
		}
	}

	virtual void GenerateHeight4(int* index_arr) {
		for(int i=0;i<4;i++) {
			GenerateHeight1(index_arr[i]);
		}
	}

	virtual void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) = 0;

	virtual bool check_vein_position(const Vector3& target_pos,EVeinType vein_type) {
		float target_height = QueryHeight(target_pos);
		return target_height < NORMAL_PLANET_RADIUS || (vein_type == EVeinType::Oil && target_height < NORMAL_PLANET_RADIUS + 0.5f);
	}

	void GenerateVeins(PlanetClassSimple& planet,const int birthPlanetId) {
		if(planet.algoId == 0)
			return;
		const ThemeProto& themeProto = LDB.Select(planet.theme);
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		int birthSeed = dotNet35Random.Next();
		DotNet35Random dotNet35Random2 = DotNet35Random(dotNet35Random.Next());
		float num = 2.1f / NORMAL_PLANET_RADIUS;
		int array[15] = {0};
		float array2[15] = {0};
		float array3[15] = {0};
		if(!themeProto.VeinSpot.empty()) {
			int copy_size = themeProto.VeinSpot.size();
			for(int i = 0; i < copy_size; ++i) {
				array[i + 1] = themeProto.VeinSpot[i];
			}
		}
		if(!themeProto.VeinCount.empty()) {
			int copy_size = themeProto.VeinCount.size();
			for(int i = 0; i < copy_size; ++i) {
				array2[i + 1] = themeProto.VeinCount[i];
			}
		}
		if(!themeProto.VeinOpacity.empty()) {
			int copy_size = themeProto.VeinOpacity.size();
			for(int i = 0; i < copy_size; ++i) {
				array3[i + 1] = themeProto.VeinOpacity[i];
			}
		}
		float p = 1.0f;
		StarClassSimple& star = *planet.star;
		ESpectrType spectr = star.spectr;
		switch(star.type)
		{
		case EStarType::MainSeqStar:
			switch(spectr)
			{
			case ESpectrType::M:
				p = 2.5f;
				break;
			case ESpectrType::K:
				p = 1.0f;
				break;
			case ESpectrType::G:
				p = 0.7f;
				break;
			case ESpectrType::F:
				p = 0.6f;
				break;
			case ESpectrType::A:
				p = 1.0f;
				break;
			case ESpectrType::B:
				p = 0.4f;
				break;
			case ESpectrType::O:
				p = 1.6f;
				break;
			}
			break;
		case EStarType::GiantStar:
			p = 2.5f;
			break;
		case EStarType::WhiteDwarf:
		{
			p = 3.5f;
			array[9]++;
			array[9]++;
			for(int j = 1; j < 12; j++)
			{
				if(dotNet35Random.NextDouble() >= 0.44999998807907104)
				{
					break;
				}
				array[9]++;
			}
			array2[9] = 0.7f;
			array3[9] = 1.0f;
			array[10]++;
			array[10]++;
			for(int k = 1; k < 12; k++)
			{
				if(dotNet35Random.NextDouble() >= 0.44999998807907104)
				{
					break;
				}
				array[10]++;
			}
			array2[10] = 0.7f;
			array3[10] = 1.0f;
			array[12]++;
			for(int l = 1; l < 12; l++)
			{
				if(dotNet35Random.NextDouble() >= 0.5)
				{
					break;
				}
				array[12]++;
			}
			array2[12] = 0.7f;
			array3[12] = 0.3f;
			break;
		}
		case EStarType::NeutronStar:
		{
			p = 4.5f;
			array[14]++;
			for(int m = 1; m < 12; m++)
			{
				if(dotNet35Random.NextDouble() >= 0.6499999761581421)
				{
					break;
				}
				array[14]++;
			}
			array2[14] = 0.7f;
			array3[14] = 0.3f;
			break;
		}
		case EStarType::BlackHole:
		{
			p = 5.0f;
			array[14]++;
			for(int i = 1; i < 12; i++)
			{
				if(dotNet35Random.NextDouble() >= 0.6499999761581421)
				{
					break;
				}
				array[14]++;
			}
			array2[14] = 0.7f;
			array3[14] = 0.3f;
			break;
		}
		}
		for(int n = 0; n < themeProto.RareVeins.size(); n++)
		{
			int num2 = themeProto.RareVeins[n];
			float num3 = ((star.index == 0) ? themeProto.RareSettings[n * 4] : themeProto.RareSettings[n * 4 + 1]);
			float num4 = themeProto.RareSettings[n * 4 + 2];
			float num5 = themeProto.RareSettings[n * 4 + 3];
			//float num6 = num5;
			num3 = 1.0f - Mathf.Pow(1.0f - num3,p);
			num5 = 1.0f - Mathf.Pow(1.0f - num5,p);
			//num6 = 1.0f - Mathf.Pow(1.0f - num6,p);
			if(!(dotNet35Random.NextDouble() < (double)num3))
			{
				continue;
			}
			array[num2]++;
			array2[num2] = num5;
			array3[num2] = num5;
			for(int num7 = 1; num7 < 12; num7++)
			{
				if(dotNet35Random.NextDouble() >= (double)num4)
				{
					break;
				}
				array[num2]++;
			}
		}
		float num8 = star.resourceCoef;
		bool flag = birthPlanetId == planet.id;
		if(flag)
			num8 *= 2.0f/3.0f;
		else if(star.galaxy->is_rare_resource) {
			if(num8 > 1.0f)
				num8 = Mathf.Pow(num8,0.8f);
			num8 *= 0.7f;
		}
		vector<Vector3> veinVectors(512);
		vector<EVeinType> veinVectorTypes(512,EVeinType::None_vein);
		vector<Vector2> tmp_vecs;
		int veinVectorCount = 0;
		Vector3 birthPoint;
		if(flag) {
			tie(birthPoint,veinVectors[0],veinVectors[1]) = GenBirthPoints(planet,birthSeed,star.uPosition);
			birthPoint.Normalize();
			birthPoint *= 0.75f;
			veinVectorTypes[0] = EVeinType::Iron;
			veinVectorTypes[1] = EVeinType::Copper;
			veinVectorCount = 2;
		} else {
			birthPoint.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.y = (float)dotNet35Random2.NextDouble() - 0.5f;
			birthPoint.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.Normalize();
			birthPoint *= (float)(dotNet35Random2.NextDouble() * 0.4 + 0.2);
		}
		for(int vein_type_index = 1; vein_type_index < 15; vein_type_index++)
		{
			if(veinVectorCount >= veinVectors.size())
			{
				break;
			}
			EVeinType eVeinType = (EVeinType)vein_type_index;
			int vein_group_num = array[vein_type_index];
			if(vein_group_num > 1)
			{
				vein_group_num += dotNet35Random2.Next(-1,2);
			}
			for(int vein_group_index = 0; vein_group_index < vein_group_num; vein_group_index++)
			{
				int try_num_1 = 0;
				Vector3 target_pos = Vector3::zero();
				bool flag2 = false;
				while(try_num_1++ < 200)
				{
					target_pos.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
					target_pos.y = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
					target_pos.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
					if(eVeinType != EVeinType::Oil)
					{
						target_pos += birthPoint;
					}
					target_pos.Normalize();
					if(check_vein_position(target_pos,eVeinType))
						continue;
					bool flag3 = false;
					float num15 = ((eVeinType == EVeinType::Oil) ? 100.0f : 196.0f);
					for(int num16 = 0; num16 < veinVectorCount; num16++)
					{
						if((veinVectors[num16] - target_pos).sqrMagnitude() < num * num * num15)
						{
							flag3 = true;
							break;
						}
					}
					if(!flag3)
					{
						flag2 = true;
						break;
					}
				}
				if(flag2)
				{
					veinVectors[veinVectorCount] = target_pos;
					veinVectorTypes[veinVectorCount] = eVeinType;
					veinVectorCount++;
					if(veinVectorCount == veinVectors.size())
					{
						break;
					}
				}
			}
		}
		for(int vein_group_index = 0; vein_group_index < veinVectorCount; vein_group_index++)
		{
			tmp_vecs.clear();
			Vector3 normalized = Vector3::Normalize(veinVectors[vein_group_index]);
			EVeinType eVeinType2 = veinVectorTypes[vein_group_index];
			int vein_point_type = (int)eVeinType2;
			glm::quat quaternion = glm::rotation(vector3_to_glm(Vector3::up()),vector3_to_glm(normalized));
			Vector3 vector = glm_to_vector3(quaternion * vector3_to_glm(Vector3::right()));
			Vector3 vector2 = glm_to_vector3(quaternion * vector3_to_glm(Vector3::forward()));
			tmp_vecs.push_back(Vector2::zero());
			int vein_point_num = Mathf.RoundToInt(array2[vein_point_type] * (float)dotNet35Random2.Next(20,25));
			if(eVeinType2 == EVeinType::Oil)
			{
				vein_point_num = 1;
			}
			float num20 = array3[vein_point_type];
			if(flag && vein_group_index < 2)
			{
				vein_point_num = 6;
				num20 = 0.2f;
			}
			int try_num_2 = 0;
			while(try_num_2++ < 20)
			{
				int count = tmp_vecs.size();
				for(int vein_point_index = 0; vein_point_index < count; vein_point_index++)
				{
					if(tmp_vecs.size() >= vein_point_num)
					{
						break;
					}
					if(tmp_vecs[vein_point_index].sqrMagnitude() > 36.0f)
					{
						continue;
					}
					double num23 = dotNet35Random2.NextDouble() * Math.PI * 2.0;
					Vector2 vector3 = Vector2((float)Math.Cos(num23),(float)Math.Sin(num23));
					vector3 += tmp_vecs[vein_point_index] * 0.2f;
					vector3.Normalize();
					Vector2 new_vein_point_pos = tmp_vecs[vein_point_index] + vector3;
					bool flag4 = false;
					for(int num24 = 0; num24 < tmp_vecs.size(); num24++)
					{
						if((tmp_vecs[num24] - new_vein_point_pos).sqrMagnitude() < 0.85f)
						{
							flag4 = true;
							break;
						}
					}
					if(!flag4)
					{
						tmp_vecs.push_back(new_vein_point_pos);
					}
				}
				if(tmp_vecs.size() >= vein_point_num)
				{
					break;
				}
			}
			float num25 = num8;
			if(eVeinType2 == EVeinType::Oil)
				num25 = Mathf.Pow(num8,0.5f);
			int num26 = Mathf.RoundToInt(num20 * 100000.0f * num25);
			if(num26 < 20)
				num26 = 20;
			int num27 = ((num26 < 16000) ? Mathf.FloorToInt((float)num26 * 0.9375f) : 15000);
			int minValue = num26 - num27;
			int maxValue = num26 + num27 + 1;
			for(int vein_point_index = 0; vein_point_index < tmp_vecs.size(); vein_point_index++)
			{
				Vector3 vector5 = (vector * tmp_vecs[vein_point_index].x + vector2 * tmp_vecs[vein_point_index].y) * num;
				int vein_amount = Mathf.RoundToInt((float)dotNet35Random2.Next(minValue,maxValue) * 1.1f);
				if(eVeinType2 != EVeinType::Oil)
					vein_amount = Mathf.RoundToInt((float)vein_amount * star.galaxy->resource_multiplier);
				else
				{
					float oil_resource_multiplier = (star.galaxy->resource_multiplier <= 0.1001f)?0.5f:1.0f;
					vein_amount = Mathf.RoundToInt((float)vein_amount * oil_resource_multiplier);
					if(vein_amount < 2500)
						vein_amount = 2500;
				}
				if(vein_amount < 1)
					vein_amount = 1;
				if(star.galaxy->resource_multiplier >= 100.0f && eVeinType2 != EVeinType::Oil)
					vein_amount = 1000000000;
				//dotNet35Random2.Next();
				Vector3 vein_pos = normalized + vector5;
				//TODO: 这里对油井坐标未变换！
				//if(vein.type == EVeinType::Oil)
				//{
				//	vein.pos = planet.aux.RawSnap(vein.pos);
				//}
				float num29 = QueryHeight(vein_pos);
				if(planet.waterItemId == 0 || num29 >= NORMAL_PLANET_RADIUS || planet.algoId == 7)
				{
					planet.veins_point[vein_point_type-1]++;
					planet.veins_amount[vein_point_type-1] += vein_amount;
				}
			}
		}
		//std::cout << "星球" << planet.id << "矿脉生成完成" << std::endl;
		tmp_vecs.clear();
	};
};

class PlanetAlgorithm0: public PlanetAlgorithm
{
public:
	void GenerateHeight1(int index) override {
		heightData[index] = (unsigned short)((double)NORMAL_PLANET_RADIUS * 100.0);
	}

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		heightData.assign(VERTICES_DATALENGTH,(unsigned short)((double)NORMAL_PLANET_RADIUS * 100.0));
	}
};

class PlanetAlgorithm1: public PlanetAlgorithm
{
public:
	static constexpr double num = 0.01;
	static constexpr double num2 = 0.012;
	static constexpr double num3 = 0.01;
	static constexpr double num4 = 3.0;
	static constexpr double num5 = -0.2;
	static constexpr double num6 = 0.9;
	static constexpr double num7 = 0.5;
	static constexpr double num8 = 2.5;
	static constexpr double num9 = 0.3;

	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num12 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num13 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num14 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num17 = simplexNoise.Noise3DFBM(num12 * num,num13 * num2,num14 * num3,6) * num4 + num5;
		double num18 = simplexNoise2.Noise3DFBM(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3) * num4 * num6 + num7;
		double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
		double num20 = num17 + num19;
		double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
		double num22 = ((num21 > 0.0) ? Maths::Levelize3(num21,0.7) : Maths::Levelize2(num21,0.5));
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num22 + 0.2) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply in float before converting, matching GenerateHeight1.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d terrain = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_mul_pd(z,_mm_set1_pd(num3)),6);
		terrain = _mm_add_pd(_mm_mul_pd(terrain,_mm_set1_pd(num4)),_mm_set1_pd(num5));
		__m128d detail = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(0.0025)),_mm_mul_pd(y,_mm_set1_pd(0.0025)),_mm_mul_pd(z,_mm_set1_pd(0.0025)),3);
		detail = _mm_add_pd(_mm_mul_pd(_mm_mul_pd(detail,_mm_set1_pd(num4)),_mm_set1_pd(num6)),_mm_set1_pd(num7));
		const __m128d zero = _mm_setzero_pd();
		detail = _mm_blendv_pd(detail,_mm_mul_pd(detail,_mm_set1_pd(0.5)),_mm_cmp_pd(detail,zero,_CMP_GT_OQ));
		__m128d height = _mm_add_pd(terrain,detail);
		height = _mm_mul_pd(height,_mm_blendv_pd(_mm_set1_pd(1.6),_mm_set1_pd(0.5),_mm_cmp_pd(height,zero,_CMP_GT_OQ)));
		const __m128d positive = _mm_cmp_pd(height,zero,_CMP_GT_OQ);
		const __m128d level = _mm_blendv_pd(_mm_set1_pd(0.5),_mm_set1_pd(0.7),positive);
		__m128d scaled = _mm_div_pd(height,level);
		__m128d integral = _mm_floor_pd(scaled);
		__m128d fraction = _mm_sub_pd(scaled,integral);
		// Levelize2 for every lane; positive heights receive a third smoothing step.
		for(int step=0;step<2;step++)
			fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		__m128d third = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		fraction = _mm_blendv_pd(fraction,third,positive);
		height = _mm_mul_pd(_mm_add_pd(integral,fraction),level);
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.2)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply in float before converting, matching GenerateHeight1.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d terrain = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_mul_pd(z,_mm256_set1_pd(num3)),6);
		terrain = _mm256_add_pd(_mm256_mul_pd(terrain,_mm256_set1_pd(num4)),_mm256_set1_pd(num5));
		__m256d detail = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.0025)),_mm256_mul_pd(y,_mm256_set1_pd(0.0025)),_mm256_mul_pd(z,_mm256_set1_pd(0.0025)),3);
		detail = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(detail,_mm256_set1_pd(num4)),_mm256_set1_pd(num6)),_mm256_set1_pd(num7));
		const __m256d zero = _mm256_setzero_pd();
		detail = _mm256_blendv_pd(detail,_mm256_mul_pd(detail,_mm256_set1_pd(0.5)),_mm256_cmp_pd(detail,zero,_CMP_GT_OQ));
		__m256d height = _mm256_add_pd(terrain,detail);
		height = _mm256_mul_pd(height,_mm256_blendv_pd(_mm256_set1_pd(1.6),_mm256_set1_pd(0.5),_mm256_cmp_pd(height,zero,_CMP_GT_OQ)));
		const __m256d positive = _mm256_cmp_pd(height,zero,_CMP_GT_OQ);
		const __m256d level = _mm256_blendv_pd(_mm256_set1_pd(0.5),_mm256_set1_pd(0.7),positive);
		__m256d scaled = _mm256_div_pd(height,level);
		__m256d integral = _mm256_floor_pd(scaled);
		__m256d fraction = _mm256_sub_pd(scaled,integral);
		// Levelize2 for every lane; positive heights receive a third smoothing step.
		for(int step=0;step<2;step++)
			fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		__m256d third = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		fraction = _mm256_blendv_pd(fraction,third,positive);
		height = _mm256_mul_pd(_mm256_add_pd(integral,fraction),level);
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.2)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num10 = dotNet35Random.Next();
		int num11 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num10);
		simplexNoise2 = SimplexNoise(num11);
		heightData.assign(VERTICES_DATALENGTH,0);
		//debugData.resize(DATALENGTH);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain1");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,tls.buffer);
			kernel.setArg(3,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm2: public PlanetAlgorithm
{
public:
	double num,num2,num3,num4;
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num8 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num9 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num10 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num14 = simplexNoise.Noise3DFBM(num8 * num,num9 * num2,num10 * num3,6,0.45,1.8);
		double value = num14 * num4 + num4 * 0.4;
		double num16 = 0.6 / (Math.Abs(value) + 0.6) - 0.25;
		double num17 = ((num16 < 0.0) ? (num16 * 0.3) : num16);
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num17 + 0.1) * 100.0);
	}
	
#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m128d noise = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_mul_pd(z,_mm_set1_pd(num3)),6,0.45,1.8);
		__m128d value = _mm_add_pd(_mm_mul_pd(noise,_mm_set1_pd(num4)),_mm_set1_pd(num4 * 0.4));
		__m128d magnitude = _mm_andnot_pd(_mm_set1_pd(-0.0),value);
		__m128d height = _mm_sub_pd(_mm_div_pd(_mm_set1_pd(0.6),_mm_add_pd(magnitude,_mm_set1_pd(0.6))),_mm_set1_pd(0.25));
		height = _mm_blendv_pd(height,_mm_mul_pd(height,_mm_set1_pd(0.3)),_mm_cmp_pd(height,_mm_setzero_pd(),_CMP_LT_OQ));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.1)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m256d noise = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_mul_pd(z,_mm256_set1_pd(num3)),6,0.45,1.8);
		__m256d value = _mm256_add_pd(_mm256_mul_pd(noise,_mm256_set1_pd(num4)),_mm256_set1_pd(num4 * 0.4));
		__m256d magnitude = _mm256_andnot_pd(_mm256_set1_pd(-0.0),value);
		__m256d height = _mm256_sub_pd(_mm256_div_pd(_mm256_set1_pd(0.6),_mm256_add_pd(magnitude,_mm256_set1_pd(0.6))),_mm256_set1_pd(0.25));
		height = _mm256_blendv_pd(height,_mm256_mul_pd(height,_mm256_set1_pd(0.3)),_mm256_cmp_pd(height,_mm256_setzero_pd(),_CMP_LT_OQ));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.1)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		double modX = planet.mod_x;
		double modY = planet.mod_y;
		modX = (3.0 - modX - modX) * modX * modX;
		num = 0.0035;
		num2 = 0.025 * modX + 0.0035 * (1.0 - modX);
		num3 = 0.0035;
		num4 = 3.0;
		double num5 = 1.0 + 1.3 * modY;
		num *= num5;
		num2 *= num5;
		num3 *= num5;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num6 = dotNet35Random.Next();
		int num7 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num6);
		simplexNoise2 = SimplexNoise(num7);
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain2");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();
			
			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,num);
			kernel.setArg(3,num2);
			kernel.setArg(4,num3);
			kernel.setArg(5,tls.buffer);
			kernel.setArg(6,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	};
};

class PlanetAlgorithm3: public PlanetAlgorithm
{
private:
	double Lerp(double a,double b,double t) {
		return a + (b - a) * t;
	}
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Lerp_batch2(__m128d a,__m128d b,__m128d t) {
		return _mm_add_pd(a,_mm_mul_pd(_mm_sub_pd(b,a),t));
	}
	__forceinline __m256d __vectorcall Lerp_batch4(__m256d a,__m256d b,__m256d t) {
		return _mm256_add_pd(a,_mm256_mul_pd(_mm256_sub_pd(b,a),t));
	}
#endif
public:
	static constexpr double num = 0.007;
	static constexpr double num2 = 0.007;
	static constexpr double num3 = 0.007;

	double modX;
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num6 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num7 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num8 = vertices[index].z * NORMAL_PLANET_RADIUS;
		num6 += Math.Sin(num7 * 0.15) * 3.0;
		num7 += Math.Sin(num8 * 0.15) * 3.0;
		num8 += Math.Sin(num6 * 0.15) * 3.0;
		double num11 = simplexNoise.Noise3DFBM(num6 * num * 1.0,num7 * num2 * 1.1,num8 * num3 * 1.0,6,0.5,1.8);
		double num12 = simplexNoise2.Noise3DFBM(num6 * num * 1.3 + 0.5,num7 * num2 * 2.8 + 0.2,num8 * num3 * 1.3 + 0.7,3) * 2.0;
		double num13 = simplexNoise2.Noise3DFBM(num6 * num * 6.0,num7 * num2 * 12.0,num8 * num3 * 6.0,2) * 2.0;
		num13 = Lerp(num13,num13 * 0.1,modX);
		double num14 = simplexNoise2.Noise3DFBM(num6 * num * 0.8,num7 * num2 * 0.8,num8 * num3 * 0.8,2) * 2.0;
		double num15 = num11 * 2.0 + 0.92;
		double num16 = num12 * (double)Mathf.Abs((float)num14 + 0.5f);
		num15 += (double)Mathf.Clamp01((float)(num16 - 0.35) * 1.0f);
		if(num15 < 0.0)
			num15 *= 2.0;
		double num17 = Maths::Levelize2(num15);
		if(num17 > 0.0) {
			num17 = Maths::Levelize2(num15);
			num17 = Lerp(Maths::Levelize4(num17),num17,modX);
		}
		double b = ((!(num17 > 0.0)) ? ((double)Mathf.Lerp(-1.0f,0.0f,(float)num17 + 1.0f)) : ((!(num17 > 1.0)) ? ((double)Mathf.Lerp(0.0f,0.3f,(float)num17) + num13 * 0.1) : ((num17 > 2.0) ? ((double)Mathf.Lerp(1.2f,2.0f,(float)num17 - 2.0f) + num13 * 0.12) : ((double)Mathf.Lerp(0.3f,1.2f,(float)num17 - 1.0f) + num13 * 0.12))));
		double a = ((!(num17 > 0.0)) ? ((double)Mathf.Lerp(-4.0f,0.0f,(float)num17 + 1.0f)) : ((!(num17 > 1.0)) ? ((double)Mathf.Lerp(0.0f,0.3f,(float)num17) + num13 * 0.1) : ((num17 > 2.0) ? ((double)Mathf.Lerp(1.4f,2.7f,(float)num17 - 2.0f) + num13 * 0.12) : ((double)Mathf.Lerp(0.3f,1.4f,(float)num17 - 1.0f) + num13 * 0.12))));
		double num18 = Lerp(a,b,modX);
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num18 + 0.2) * 100.0);
		//data.debugData[i] = num18;
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		// AVX2 has no sine instruction. Keep the existing libm sine and warp order.
		auto sine = [](__m128d value) {
			double lanes[2];
			_mm_storeu_pd(lanes,value);
			for(int lane=0;lane<2;lane++)
				lanes[lane] = Math.Sin(lanes[lane]);
			return _mm_loadu_pd(lanes);
		};
		x = _mm_add_pd(x,_mm_mul_pd(sine(_mm_mul_pd(y,_mm_set1_pd(0.15))),_mm_set1_pd(3.0)));
		y = _mm_add_pd(y,_mm_mul_pd(sine(_mm_mul_pd(z,_mm_set1_pd(0.15))),_mm_set1_pd(3.0)));
		z = _mm_add_pd(z,_mm_mul_pd(sine(_mm_mul_pd(x,_mm_set1_pd(0.15))),_mm_set1_pd(3.0)));
		__m128d fx = _mm_mul_pd(x,_mm_set1_pd(num));
		__m128d fy = _mm_mul_pd(y,_mm_set1_pd(num2));
		__m128d fz = _mm_mul_pd(z,_mm_set1_pd(num3));
		__m128d base = simplexNoise.Noise3DFBM_batch2(fx,_mm_mul_pd(fy,_mm_set1_pd(1.1)),fz,6,0.5,1.8);
		__m128d detail = simplexNoise2.Noise3DFBM_batch2(_mm_add_pd(_mm_mul_pd(fx,_mm_set1_pd(1.3)),_mm_set1_pd(0.5)),_mm_add_pd(_mm_mul_pd(fy,_mm_set1_pd(2.8)),_mm_set1_pd(0.2)),_mm_add_pd(_mm_mul_pd(fz,_mm_set1_pd(1.3)),_mm_set1_pd(0.7)),3);
		detail = _mm_mul_pd(detail,_mm_set1_pd(2.0));
		__m128d roughness = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(fx,_mm_set1_pd(6.0)),_mm_mul_pd(fy,_mm_set1_pd(12.0)),_mm_mul_pd(fz,_mm_set1_pd(6.0)),2);
		roughness = _mm_mul_pd(roughness,_mm_set1_pd(2.0));
		roughness = Lerp_batch2(roughness,_mm_mul_pd(roughness,_mm_set1_pd(0.1)),_mm_set1_pd(modX));
		__m128d envelope = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(fx,_mm_set1_pd(0.8)),_mm_mul_pd(fy,_mm_set1_pd(0.8)),_mm_mul_pd(fz,_mm_set1_pd(0.8)),2);
		envelope = _mm_mul_pd(envelope,_mm_set1_pd(2.0));
		// Preserve Mathf's float rounding, clamping and interpolation.
		__m128 envelopeFloat = _mm_andnot_ps(_mm_set1_ps(-0.0f),_mm_add_ps(_mm_cvtpd_ps(envelope),_mm_set1_ps(0.5f)));
		__m128d modulation = _mm_mul_pd(detail,_mm_cvtps_pd(envelopeFloat));
		__m128 clamped = _mm_min_ps(_mm_set1_ps(1.0f),_mm_max_ps(_mm_setzero_ps(),_mm_cvtpd_ps(_mm_sub_pd(modulation,_mm_set1_pd(0.35)))));
		__m128d terrain = _mm_add_pd(_mm_add_pd(_mm_mul_pd(base,_mm_set1_pd(2.0)),_mm_set1_pd(0.92)),_mm_cvtps_pd(clamped));
		terrain = _mm_blendv_pd(terrain,_mm_mul_pd(terrain,_mm_set1_pd(2.0)),_mm_cmp_pd(terrain,_mm_setzero_pd(),_CMP_LT_OQ));
		auto levelize = [](__m128d value,int steps) {
			__m128d integral = _mm_floor_pd(value);
			__m128d fraction = _mm_sub_pd(value,integral);
			for(int step=0;step<steps;step++)
				fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
			return _mm_add_pd(integral,fraction);
		};
		__m128d shaped = levelize(terrain,2);
		__m128d terraces = levelize(shaped,4);
		__m128d interpolated = Lerp_batch2(terraces,shaped,_mm_set1_pd(modX));
		shaped = _mm_blendv_pd(shaped,interpolated,_mm_cmp_pd(shaped,_mm_setzero_pd(),_CMP_GT_OQ));
		auto lerpFloat = [](float a,float b,__m128 t) {
			t = _mm_min_ps(_mm_set1_ps(1.0f),_mm_max_ps(_mm_setzero_ps(),t));
			return _mm_add_ps(_mm_set1_ps(a),_mm_mul_ps(_mm_set1_ps(b-a),t));
		};
		__m128 shapedFloat = _mm_cvtpd_ps(shaped);
		__m128d lowA = _mm_cvtps_pd(lerpFloat(-4.0f,0.0f,_mm_add_ps(shapedFloat,_mm_set1_ps(1.0f))));
		__m128d lowB = _mm_cvtps_pd(lerpFloat(-1.0f,0.0f,_mm_add_ps(shapedFloat,_mm_set1_ps(1.0f))));
		__m128d middle = _mm_add_pd(_mm_cvtps_pd(lerpFloat(0.0f,0.3f,shapedFloat)),_mm_mul_pd(roughness,_mm_set1_pd(0.1)));
		__m128d upperA = _mm_add_pd(_mm_cvtps_pd(lerpFloat(0.3f,1.4f,_mm_sub_ps(shapedFloat,_mm_set1_ps(1.0f)))),_mm_mul_pd(roughness,_mm_set1_pd(0.12)));
		__m128d upperB = _mm_add_pd(_mm_cvtps_pd(lerpFloat(0.3f,1.2f,_mm_sub_ps(shapedFloat,_mm_set1_ps(1.0f)))),_mm_mul_pd(roughness,_mm_set1_pd(0.12)));
		__m128d highA = _mm_add_pd(_mm_cvtps_pd(lerpFloat(1.4f,2.7f,_mm_sub_ps(shapedFloat,_mm_set1_ps(2.0f)))),_mm_mul_pd(roughness,_mm_set1_pd(0.12)));
		__m128d highB = _mm_add_pd(_mm_cvtps_pd(lerpFloat(1.2f,2.0f,_mm_sub_ps(shapedFloat,_mm_set1_ps(2.0f)))),_mm_mul_pd(roughness,_mm_set1_pd(0.12)));
		__m128d aboveTwo = _mm_cmp_pd(shaped,_mm_set1_pd(2.0),_CMP_GT_OQ);
		__m128d aboveOne = _mm_cmp_pd(shaped,_mm_set1_pd(1.0),_CMP_GT_OQ);
		__m128d positive = _mm_cmp_pd(shaped,_mm_setzero_pd(),_CMP_GT_OQ);
		__m128d a = _mm_blendv_pd(lowA,_mm_blendv_pd(middle,_mm_blendv_pd(upperA,highA,aboveTwo),aboveOne),positive);
		__m128d b = _mm_blendv_pd(lowB,_mm_blendv_pd(middle,_mm_blendv_pd(upperB,highB,aboveTwo),aboveOne),positive);
		__m128d height = Lerp_batch2(a,b,_mm_set1_pd(modX));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.2)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		
		// AVX2 has no sine instruction. Keep the existing libm sine and warp order.
		auto sine = [](__m256d value) {
			double lanes[4];
			_mm256_storeu_pd(lanes,value);
			for(int lane=0;lane<4;lane++)
				lanes[lane] = Math.Sin(lanes[lane]);
			return _mm256_loadu_pd(lanes);
		};
		x = _mm256_add_pd(x,_mm256_mul_pd(sine(_mm256_mul_pd(y,_mm256_set1_pd(0.15))),_mm256_set1_pd(3.0)));
		y = _mm256_add_pd(y,_mm256_mul_pd(sine(_mm256_mul_pd(z,_mm256_set1_pd(0.15))),_mm256_set1_pd(3.0)));
		z = _mm256_add_pd(z,_mm256_mul_pd(sine(_mm256_mul_pd(x,_mm256_set1_pd(0.15))),_mm256_set1_pd(3.0)));
		__m256d fx = _mm256_mul_pd(x,_mm256_set1_pd(num));
		__m256d fy = _mm256_mul_pd(y,_mm256_set1_pd(num2));
		__m256d fz = _mm256_mul_pd(z,_mm256_set1_pd(num3));
		__m256d base = simplexNoise.Noise3DFBM_batch4(fx,_mm256_mul_pd(fy,_mm256_set1_pd(1.1)),fz,6,0.5,1.8);
		__m256d detail = simplexNoise2.Noise3DFBM_batch4(_mm256_add_pd(_mm256_mul_pd(fx,_mm256_set1_pd(1.3)),_mm256_set1_pd(0.5)),_mm256_add_pd(_mm256_mul_pd(fy,_mm256_set1_pd(2.8)),_mm256_set1_pd(0.2)),_mm256_add_pd(_mm256_mul_pd(fz,_mm256_set1_pd(1.3)),_mm256_set1_pd(0.7)),3);
		detail = _mm256_mul_pd(detail,_mm256_set1_pd(2.0));
		__m256d roughness = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(fx,_mm256_set1_pd(6.0)),_mm256_mul_pd(fy,_mm256_set1_pd(12.0)),_mm256_mul_pd(fz,_mm256_set1_pd(6.0)),2);
		roughness = _mm256_mul_pd(roughness,_mm256_set1_pd(2.0));
		roughness = Lerp_batch4(roughness,_mm256_mul_pd(roughness,_mm256_set1_pd(0.1)),_mm256_set1_pd(modX));
		__m256d envelope = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(fx,_mm256_set1_pd(0.8)),_mm256_mul_pd(fy,_mm256_set1_pd(0.8)),_mm256_mul_pd(fz,_mm256_set1_pd(0.8)),2);
		envelope = _mm256_mul_pd(envelope,_mm256_set1_pd(2.0));
		// Preserve Mathf's float rounding, clamping and interpolation.
		__m128 envelopeFloat = _mm_andnot_ps(_mm_set1_ps(-0.0f),_mm_add_ps(_mm256_cvtpd_ps(envelope),_mm_set1_ps(0.5f)));
		__m256d modulation = _mm256_mul_pd(detail,_mm256_cvtps_pd(envelopeFloat));
		__m128 clamped = _mm_min_ps(_mm_set1_ps(1.0f),_mm_max_ps(_mm_setzero_ps(),_mm256_cvtpd_ps(_mm256_sub_pd(modulation,_mm256_set1_pd(0.35)))));
		__m256d terrain = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(base,_mm256_set1_pd(2.0)),_mm256_set1_pd(0.92)),_mm256_cvtps_pd(clamped));
		terrain = _mm256_blendv_pd(terrain,_mm256_mul_pd(terrain,_mm256_set1_pd(2.0)),_mm256_cmp_pd(terrain,_mm256_setzero_pd(),_CMP_LT_OQ));
		auto levelize = [](__m256d value,int steps) {
			__m256d integral = _mm256_floor_pd(value);
			__m256d fraction = _mm256_sub_pd(value,integral);
			for(int step=0;step<steps;step++)
				fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
			return _mm256_add_pd(integral,fraction);
		};
		__m256d shaped = levelize(terrain,2);
		__m256d terraces = levelize(shaped,4);
		__m256d interpolated = Lerp_batch4(terraces,shaped,_mm256_set1_pd(modX));
		shaped = _mm256_blendv_pd(shaped,interpolated,_mm256_cmp_pd(shaped,_mm256_setzero_pd(),_CMP_GT_OQ));
		auto lerpFloat = [](float a,float b,__m128 t) {
			t = _mm_min_ps(_mm_set1_ps(1.0f),_mm_max_ps(_mm_setzero_ps(),t));
			return _mm_add_ps(_mm_set1_ps(a),_mm_mul_ps(_mm_set1_ps(b-a),t));
		};
		__m128 shapedFloat = _mm256_cvtpd_ps(shaped);
		__m256d lowA = _mm256_cvtps_pd(lerpFloat(-4.0f,0.0f,_mm_add_ps(shapedFloat,_mm_set1_ps(1.0f))));
		__m256d lowB = _mm256_cvtps_pd(lerpFloat(-1.0f,0.0f,_mm_add_ps(shapedFloat,_mm_set1_ps(1.0f))));
		__m256d middle = _mm256_add_pd(_mm256_cvtps_pd(lerpFloat(0.0f,0.3f,shapedFloat)),_mm256_mul_pd(roughness,_mm256_set1_pd(0.1)));
		__m256d upperA = _mm256_add_pd(_mm256_cvtps_pd(lerpFloat(0.3f,1.4f,_mm_sub_ps(shapedFloat,_mm_set1_ps(1.0f)))),_mm256_mul_pd(roughness,_mm256_set1_pd(0.12)));
		__m256d upperB = _mm256_add_pd(_mm256_cvtps_pd(lerpFloat(0.3f,1.2f,_mm_sub_ps(shapedFloat,_mm_set1_ps(1.0f)))),_mm256_mul_pd(roughness,_mm256_set1_pd(0.12)));
		__m256d highA = _mm256_add_pd(_mm256_cvtps_pd(lerpFloat(1.4f,2.7f,_mm_sub_ps(shapedFloat,_mm_set1_ps(2.0f)))),_mm256_mul_pd(roughness,_mm256_set1_pd(0.12)));
		__m256d highB = _mm256_add_pd(_mm256_cvtps_pd(lerpFloat(1.2f,2.0f,_mm_sub_ps(shapedFloat,_mm_set1_ps(2.0f)))),_mm256_mul_pd(roughness,_mm256_set1_pd(0.12)));
		__m256d aboveTwo = _mm256_cmp_pd(shaped,_mm256_set1_pd(2.0),_CMP_GT_OQ);
		__m256d aboveOne = _mm256_cmp_pd(shaped,_mm256_set1_pd(1.0),_CMP_GT_OQ);
		__m256d positive = _mm256_cmp_pd(shaped,_mm256_setzero_pd(),_CMP_GT_OQ);
		__m256d a = _mm256_blendv_pd(lowA,_mm256_blendv_pd(middle,_mm256_blendv_pd(upperA,highA,aboveTwo),aboveOne),positive);
		__m256d b = _mm256_blendv_pd(lowB,_mm256_blendv_pd(middle,_mm256_blendv_pd(upperB,highB,aboveTwo),aboveOne),positive);
		__m256d height = Lerp_batch4(a,b,_mm256_set1_pd(modX));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.2)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		modX = planet.mod_x;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num4);
		simplexNoise2 = SimplexNoise(num5);
		heightData.assign(VERTICES_DATALENGTH,0);
		//data.debugData.resize(DATALENGTH);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain3");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,modX);
			kernel.setArg(3,tls.buffer);
			kernel.setArg(4,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm4: public PlanetAlgorithm
{
public:
	static constexpr int kCircleCount = 80;
	static constexpr double num = 0.007;
	static constexpr double num2 = 0.007;
	static constexpr double num3 = 0.007;

	Vector4 circles[80];
	double heights[80];
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num7 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num8 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num9 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num12 = simplexNoise.Noise3DFBM(num7 * num,num8 * num2,num9 * num3,4,0.45,1.8);
		double num13 = simplexNoise2.Noise3DFBM(num7 * num * 5.0,num8 * num2 * 5.0,num9 * num3 * 5.0,4);
		double num14 = num12 * 1.5;
		double num15 = num13 * 0.2;
		double num16 = num14 * 0.08 + num15 * 2.0;
		double num17 = 0.0;
		for(int k = 0; k < 80; k++) {
			double num18 = (double)circles[k].x - num7;
			double num19 = (double)circles[k].y - num8;
			double num20 = (double)circles[k].z - num9;
			double num21 = num18 * num18 + num19 * num19 + num20 * num20;
			if(num21 <= (double)circles[k].w) {
				double num22 = num21 / (double)circles[k].w + num15 * 1.2;
				if(num22 < 0.0)
					num22 = 0.0;
				double num23 = num22 * num22;
				double num24 = num23 * num22;
				double num25 = -15.0 * num24 + 21.833333333334 * num23 - 7.533333333333 * num22 + 0.7 + num15;
				if(num25 < 0.0)
					num25 = 0.0;
				num25 *= num25;
				num25 *= heights[k];
				num17 = ((num17 > num25) ? num17 : num25);
			}
		}
		double num10 = num17 + num16 + 0.2;
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num10 + 0.1) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m128d fx = _mm_mul_pd(x,_mm_set1_pd(num));
		__m128d fy = _mm_mul_pd(y,_mm_set1_pd(num2));
		__m128d fz = _mm_mul_pd(z,_mm_set1_pd(num3));
		__m128d base = simplexNoise.Noise3DFBM_batch2(fx,fy,fz,4,0.45,1.8);
		__m128d detail = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(fx,_mm_set1_pd(5.0)),_mm_mul_pd(fy,_mm_set1_pd(5.0)),_mm_mul_pd(fz,_mm_set1_pd(5.0)),4);
		base = _mm_mul_pd(base,_mm_set1_pd(1.5));
		detail = _mm_mul_pd(detail,_mm_set1_pd(0.2));
		__m128d background = _mm_add_pd(_mm_mul_pd(base,_mm_set1_pd(0.08)),_mm_mul_pd(detail,_mm_set1_pd(2.0)));
		__m128d peak = _mm_setzero_pd();
		for(int k=0;k<kCircleCount;k++) {
			__m128d dx = _mm_sub_pd(_mm_set1_pd((double)circles[k].x),x);
			__m128d dy = _mm_sub_pd(_mm_set1_pd((double)circles[k].y),y);
			__m128d dz = _mm_sub_pd(_mm_set1_pd((double)circles[k].z),z);
			__m128d distance = _mm_add_pd(_mm_add_pd(_mm_mul_pd(dx,dx),_mm_mul_pd(dy,dy)),_mm_mul_pd(dz,dz));
			__m128d width = _mm_set1_pd((double)circles[k].w);
			__m128d inside = _mm_cmp_pd(distance,width,_CMP_LE_OQ);
			// Most circles miss every lane; avoid evaluating their polynomial.
			if(_mm_movemask_pd(inside) == 0)
				continue;
			__m128d t = _mm_add_pd(_mm_div_pd(distance,width),_mm_mul_pd(detail,_mm_set1_pd(1.2)));
			t = _mm_max_pd(_mm_setzero_pd(),t);
			__m128d squared = _mm_mul_pd(t,t);
			__m128d cubed = _mm_mul_pd(squared,t);
			__m128d value = _mm_add_pd(_mm_add_pd(_mm_sub_pd(_mm_add_pd(_mm_mul_pd(_mm_set1_pd(-15.0),cubed),_mm_mul_pd(_mm_set1_pd(21.833333333334),squared)),_mm_mul_pd(_mm_set1_pd(7.533333333333),t)),_mm_set1_pd(0.7)),detail);
			value = _mm_max_pd(_mm_setzero_pd(),value);
			value = _mm_mul_pd(_mm_mul_pd(value,value),_mm_set1_pd(heights[k]));
			peak = _mm_blendv_pd(peak,_mm_max_pd(peak,value),inside);
		}
		__m128d height = _mm_add_pd(_mm_add_pd(peak,background),_mm_set1_pd(0.2));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.1)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m256d fx = _mm256_mul_pd(x,_mm256_set1_pd(num));
		__m256d fy = _mm256_mul_pd(y,_mm256_set1_pd(num2));
		__m256d fz = _mm256_mul_pd(z,_mm256_set1_pd(num3));
		__m256d base = simplexNoise.Noise3DFBM_batch4(fx,fy,fz,4,0.45,1.8);
		__m256d detail = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(fx,_mm256_set1_pd(5.0)),_mm256_mul_pd(fy,_mm256_set1_pd(5.0)),_mm256_mul_pd(fz,_mm256_set1_pd(5.0)),4);
		base = _mm256_mul_pd(base,_mm256_set1_pd(1.5));
		detail = _mm256_mul_pd(detail,_mm256_set1_pd(0.2));
		__m256d background = _mm256_add_pd(_mm256_mul_pd(base,_mm256_set1_pd(0.08)),_mm256_mul_pd(detail,_mm256_set1_pd(2.0)));
		__m256d peak = _mm256_setzero_pd();
		for(int k=0;k<kCircleCount;k++) {
			__m256d dx = _mm256_sub_pd(_mm256_set1_pd((double)circles[k].x),x);
			__m256d dy = _mm256_sub_pd(_mm256_set1_pd((double)circles[k].y),y);
			__m256d dz = _mm256_sub_pd(_mm256_set1_pd((double)circles[k].z),z);
			__m256d distance = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx,dx),_mm256_mul_pd(dy,dy)),_mm256_mul_pd(dz,dz));
			__m256d width = _mm256_set1_pd((double)circles[k].w);
			__m256d inside = _mm256_cmp_pd(distance,width,_CMP_LE_OQ);
			// Most circles miss every lane; avoid evaluating their polynomial.
			if(_mm256_movemask_pd(inside) == 0)
				continue;
			__m256d t = _mm256_add_pd(_mm256_div_pd(distance,width),_mm256_mul_pd(detail,_mm256_set1_pd(1.2)));
			t = _mm256_max_pd(_mm256_setzero_pd(),t);
			__m256d squared = _mm256_mul_pd(t,t);
			__m256d cubed = _mm256_mul_pd(squared,t);
			__m256d value = _mm256_add_pd(_mm256_add_pd(_mm256_sub_pd(_mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(-15.0),cubed),_mm256_mul_pd(_mm256_set1_pd(21.833333333334),squared)),_mm256_mul_pd(_mm256_set1_pd(7.533333333333),t)),_mm256_set1_pd(0.7)),detail);
			value = _mm256_max_pd(_mm256_setzero_pd(),value);
			value = _mm256_mul_pd(_mm256_mul_pd(value,value),_mm256_set1_pd(heights[k]));
			peak = _mm256_blendv_pd(peak,_mm256_max_pd(peak,value),inside);
		}
		__m256d height = _mm256_add_pd(_mm256_add_pd(peak,background),_mm256_set1_pd(0.2));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.1)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num4);
		simplexNoise2 = SimplexNoise(num5);
		int num6 = dotNet35Random.Next();
		for(int i = 0; i < 80; i++) {
			VectorLF3 vectorLF = RandomTable::SphericNormal(num6,1.0);
			Vector4 vector = Vector4((float)vectorLF.x,(float)vectorLF.y,(float)vectorLF.z);
			vector.Normalize();
			vector *= NORMAL_PLANET_RADIUS;
			vector.w = (float)vectorLF.magnitude() * 8.0f + 8.0f;
			vector.w *= vector.w;
			circles[i] = vector;
			heights[i] = dotNet35Random.NextDouble() * 0.4 + 0.20000000298023224;
		}

		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain4");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.double_buffer(),heights,sizeof(heights));
			memcpy(tls.float_buffer(),circles,sizeof(circles));
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,tls.buffer);
			kernel.setArg(3,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm5: public PlanetAlgorithm
{
private:
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Levelize_batch2(__m128d value,double level = 1.0) {
		value = _mm_div_pd(value,_mm_set1_pd(level));
		__m128d integral = _mm_floor_pd(value);
		__m128d fraction = _mm_sub_pd(value,integral);
		fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm_mul_pd(_mm_add_pd(integral,fraction),_mm_set1_pd(level));
	}
	__forceinline __m256d __vectorcall Levelize_batch4(__m256d value,double level = 1.0) {
		value = _mm256_div_pd(value,_mm256_set1_pd(level));
		__m256d integral = _mm256_floor_pd(value);
		__m256d fraction = _mm256_sub_pd(value,integral);
		fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm256_mul_pd(_mm256_add_pd(integral,fraction),_mm256_set1_pd(level));
	}
#endif
public:
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num3 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num4 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num5 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num8 = Maths::Levelize(num3 * 0.007);
		double num9 = Maths::Levelize(num4 * 0.007);
		double num10 = Maths::Levelize(num5 * 0.007);
		num8 += simplexNoise.Noise(num3 * 0.05,num4 * 0.05,num5 * 0.05) * 0.04;
		num9 += simplexNoise.Noise(num4 * 0.05,num5 * 0.05,num3 * 0.05) * 0.04;
		num10 += simplexNoise.Noise(num5 * 0.05,num3 * 0.05,num4 * 0.05) * 0.04;
		double num11 = Math.Abs(simplexNoise2.Noise(num8,num9,num10));
		double num12 = (0.16 - num11) * 10.0;
		num12 = ((!(num12 > 0.0)) ? 0.0 : ((num12 > 1.0) ? 1.0 : num12));
		num12 *= num12;
		double num13 = (simplexNoise.Noise3DFBM(num4 * 0.005,num5 * 0.005,num3 * 0.005,4) + 0.22) * 5.0;
		num13 = ((!(num13 > 0.0)) ? 0.0 : ((num13 > 1.0) ? 1.0 : num13));
		double num14 = Math.Abs(simplexNoise2.Noise3DFBM(num8 * 1.5,num9 * 1.5,num10 * 1.5,2));
		double num6 = num12 * -1.2 * num13;
		if(num6 >= 0.0)
			num6 += num11 * 0.25 + num14 * 0.6;
		num6 -= 0.1;
		double num16 = -0.3 - num6;
		if(num16 > 0.0) {
			double num17 = simplexNoise2.Noise(num3 * 0.16,num4 * 0.16,num5 * 0.16) - 1.0;
			num16 = ((num16 > 1.0) ? 1.0 : num16);
			num16 = (3.0 - num16 - num16) * num16 * num16;
			num6 = -0.3 - num16 * 3.700000047683716 + num16 * num16 * num16 * num16 * num17 * 0.5;
		}
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num6 + 0.2) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m128d warpedX = Levelize_batch2(_mm_mul_pd(x,_mm_set1_pd(0.007)));
		__m128d warpedY = Levelize_batch2(_mm_mul_pd(y,_mm_set1_pd(0.007)));
		__m128d warpedZ = Levelize_batch2(_mm_mul_pd(z,_mm_set1_pd(0.007)));
		__m128d fx = _mm_mul_pd(x,_mm_set1_pd(0.05));
		__m128d fy = _mm_mul_pd(y,_mm_set1_pd(0.05));
		__m128d fz = _mm_mul_pd(z,_mm_set1_pd(0.05));
		warpedX = _mm_add_pd(warpedX,_mm_mul_pd(simplexNoise.Noise_batch2(fx,fy,fz),_mm_set1_pd(0.04)));
		warpedY = _mm_add_pd(warpedY,_mm_mul_pd(simplexNoise.Noise_batch2(fy,fz,fx),_mm_set1_pd(0.04)));
		warpedZ = _mm_add_pd(warpedZ,_mm_mul_pd(simplexNoise.Noise_batch2(fz,fx,fy),_mm_set1_pd(0.04)));
		const __m128d zero = _mm_setzero_pd();
		const __m128d one = _mm_set1_pd(1.0);
		const __m128d sign = _mm_set1_pd(-0.0);
		__m128d noise = _mm_andnot_pd(sign,simplexNoise2.Noise_batch2(warpedX,warpedY,warpedZ));
		__m128d ridge = _mm_mul_pd(_mm_sub_pd(_mm_set1_pd(0.16),noise),_mm_set1_pd(10.0));
		ridge = _mm_and_pd(_mm_min_pd(ridge,one),_mm_cmp_pd(ridge,zero,_CMP_GT_OQ));
		ridge = _mm_mul_pd(ridge,ridge);
		__m128d envelope = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(y,_mm_set1_pd(0.005)),_mm_mul_pd(z,_mm_set1_pd(0.005)),_mm_mul_pd(x,_mm_set1_pd(0.005)),4);
		envelope = _mm_mul_pd(_mm_add_pd(envelope,_mm_set1_pd(0.22)),_mm_set1_pd(5.0));
		envelope = _mm_and_pd(_mm_min_pd(envelope,one),_mm_cmp_pd(envelope,zero,_CMP_GT_OQ));
		__m128d detail = _mm_andnot_pd(sign,simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(warpedX,_mm_set1_pd(1.5)),_mm_mul_pd(warpedY,_mm_set1_pd(1.5)),_mm_mul_pd(warpedZ,_mm_set1_pd(1.5)),2));
		__m128d height = _mm_mul_pd(_mm_mul_pd(ridge,_mm_set1_pd(-1.2)),envelope);
		__m128d raised = _mm_add_pd(height,_mm_add_pd(_mm_mul_pd(noise,_mm_set1_pd(0.25)),_mm_mul_pd(detail,_mm_set1_pd(0.6))));
		height = _mm_blendv_pd(height,raised,_mm_cmp_pd(height,zero,_CMP_GE_OQ));
		height = _mm_sub_pd(height,_mm_set1_pd(0.1));
		__m128d depth = _mm_sub_pd(_mm_set1_pd(-0.3),height);
		__m128d depressed = _mm_cmp_pd(depth,zero,_CMP_GT_OQ);

		// Skip the additional noise when no lane enters the depression branch.
		if(_mm_movemask_pd(depressed) != 0) {
			__m128d erosion = _mm_sub_pd(simplexNoise2.Noise_batch2(_mm_mul_pd(x,_mm_set1_pd(0.16)),_mm_mul_pd(y,_mm_set1_pd(0.16)),_mm_mul_pd(z,_mm_set1_pd(0.16))),one);
			depth = _mm_min_pd(depth,one);
			depth = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),depth),depth),depth),depth);
			// Keep left-to-right multiplication, including the fourth power.
			__m128d erosionHeight = _mm_mul_pd(_mm_mul_pd(_mm_mul_pd(_mm_mul_pd(_mm_mul_pd(depth,depth),depth),depth),erosion),_mm_set1_pd(0.5));
			__m128d lowered = _mm_add_pd(_mm_sub_pd(_mm_set1_pd(-0.3),_mm_mul_pd(depth,_mm_set1_pd(3.700000047683716))),erosionHeight);
			height = _mm_blendv_pd(height,lowered,depressed);
		}
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.2)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m256d warpedX = Levelize_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.007)));
		__m256d warpedY = Levelize_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.007)));
		__m256d warpedZ = Levelize_batch4(_mm256_mul_pd(z,_mm256_set1_pd(0.007)));
		__m256d fx = _mm256_mul_pd(x,_mm256_set1_pd(0.05));
		__m256d fy = _mm256_mul_pd(y,_mm256_set1_pd(0.05));
		__m256d fz = _mm256_mul_pd(z,_mm256_set1_pd(0.05));
		warpedX = _mm256_add_pd(warpedX,_mm256_mul_pd(simplexNoise.Noise_batch4(fx,fy,fz),_mm256_set1_pd(0.04)));
		warpedY = _mm256_add_pd(warpedY,_mm256_mul_pd(simplexNoise.Noise_batch4(fy,fz,fx),_mm256_set1_pd(0.04)));
		warpedZ = _mm256_add_pd(warpedZ,_mm256_mul_pd(simplexNoise.Noise_batch4(fz,fx,fy),_mm256_set1_pd(0.04)));
		const __m256d zero = _mm256_setzero_pd();
		const __m256d one = _mm256_set1_pd(1.0);
		const __m256d sign = _mm256_set1_pd(-0.0);
		__m256d noise = _mm256_andnot_pd(sign,simplexNoise2.Noise_batch4(warpedX,warpedY,warpedZ));
		__m256d ridge = _mm256_mul_pd(_mm256_sub_pd(_mm256_set1_pd(0.16),noise),_mm256_set1_pd(10.0));
		ridge = _mm256_and_pd(_mm256_min_pd(ridge,one),_mm256_cmp_pd(ridge,zero,_CMP_GT_OQ));
		ridge = _mm256_mul_pd(ridge,ridge);
		__m256d envelope = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.005)),_mm256_mul_pd(z,_mm256_set1_pd(0.005)),_mm256_mul_pd(x,_mm256_set1_pd(0.005)),4);
		envelope = _mm256_mul_pd(_mm256_add_pd(envelope,_mm256_set1_pd(0.22)),_mm256_set1_pd(5.0));
		envelope = _mm256_and_pd(_mm256_min_pd(envelope,one),_mm256_cmp_pd(envelope,zero,_CMP_GT_OQ));
		__m256d detail = _mm256_andnot_pd(sign,simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(warpedX,_mm256_set1_pd(1.5)),_mm256_mul_pd(warpedY,_mm256_set1_pd(1.5)),_mm256_mul_pd(warpedZ,_mm256_set1_pd(1.5)),2));
		__m256d height = _mm256_mul_pd(_mm256_mul_pd(ridge,_mm256_set1_pd(-1.2)),envelope);
		__m256d raised = _mm256_add_pd(height,_mm256_add_pd(_mm256_mul_pd(noise,_mm256_set1_pd(0.25)),_mm256_mul_pd(detail,_mm256_set1_pd(0.6))));
		height = _mm256_blendv_pd(height,raised,_mm256_cmp_pd(height,zero,_CMP_GE_OQ));
		height = _mm256_sub_pd(height,_mm256_set1_pd(0.1));
		__m256d depth = _mm256_sub_pd(_mm256_set1_pd(-0.3),height);
		__m256d depressed = _mm256_cmp_pd(depth,zero,_CMP_GT_OQ);

		// Skip the additional noise when no lane enters the depression branch.
		if(_mm256_movemask_pd(depressed) != 0) {
			__m256d erosion = _mm256_sub_pd(simplexNoise2.Noise_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.16)),_mm256_mul_pd(y,_mm256_set1_pd(0.16)),_mm256_mul_pd(z,_mm256_set1_pd(0.16))),one);
			depth = _mm256_min_pd(depth,one);
			depth = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),depth),depth),depth),depth);
			// Keep left-to-right multiplication, including the fourth power.
			__m256d erosionHeight = _mm256_mul_pd(_mm256_mul_pd(_mm256_mul_pd(_mm256_mul_pd(_mm256_mul_pd(depth,depth),depth),depth),erosion),_mm256_set1_pd(0.5));
			__m256d lowered = _mm256_add_pd(_mm256_sub_pd(_mm256_set1_pd(-0.3),_mm256_mul_pd(depth,_mm256_set1_pd(3.700000047683716))),erosionHeight);
			height = _mm256_blendv_pd(height,lowered,depressed);
		}
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.2)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num = dotNet35Random.Next();
		int num2 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num);
		simplexNoise2 = SimplexNoise(num2);
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain5");
			
			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,tls.buffer);
			kernel.setArg(3,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm6: public PlanetAlgorithm
{
private:
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Levelize_batch2(__m128d value,double level = 1.0) {
		value = _mm_div_pd(value,_mm_set1_pd(level));
		__m128d integral = _mm_floor_pd(value);
		__m128d fraction = _mm_sub_pd(value,integral);
		fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm_mul_pd(_mm_add_pd(integral,fraction),_mm_set1_pd(level));
	}
	__forceinline __m256d __vectorcall Levelize_batch4(__m256d value,double level = 1.0) {
		value = _mm256_div_pd(value,_mm256_set1_pd(level));
		__m256d integral = _mm256_floor_pd(value);
		__m256d fraction = _mm256_sub_pd(value,integral);
		fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm256_mul_pd(_mm256_add_pd(integral,fraction),_mm256_set1_pd(level));
	}
#endif
public:
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num3 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num4 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num5 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num8 = Maths::Levelize(num3 * 0.007);
		double num9 = Maths::Levelize(num4 * 0.007);
		double num10 = Maths::Levelize(num5 * 0.007);
		num8 += simplexNoise.Noise(num3 * 0.05,num4 * 0.05,num5 * 0.05) * 0.04;
		num9 += simplexNoise.Noise(num4 * 0.05,num5 * 0.05,num3 * 0.05) * 0.04;
		num10 += simplexNoise.Noise(num5 * 0.05,num3 * 0.05,num4 * 0.05) * 0.04;
		double num11 = Math.Abs(simplexNoise2.Noise(num8,num9,num10));
		double num12 = (0.16 - num11) * 10.0;
		num12 = ((!(num12 > 0.0)) ? 0.0 : ((num12 > 1.0) ? 1.0 : num12));
		num12 *= num12;
		double num13 = (simplexNoise.Noise3DFBM(num4 * 0.005,num5 * 0.005,num3 * 0.005,4) + 0.22) * 5.0;
		num13 = ((!(num13 > 0.0)) ? 0.0 : ((num13 > 1.0) ? 1.0 : num13));
		double num14 = Math.Abs(simplexNoise2.Noise3DFBM(num8 * 1.5,num9 * 1.5,num10 * 1.5,2));
		double num6 = num12 * -1.2 * num13;
		if(num6 >= 0.0)
			num6 += num11 * 0.25 + num14 * 0.6;
		num6 -= 0.1;
		double num15 = -0.3 - num6;
		if(num15 > 0.0) {
			num15 = ((num15 > 1.0) ? 1.0 : num15);
			num15 = (3.0 - num15 - num15) * num15 * num15;
			num6 = -0.3 - num15 * 3.700000047683716;
		}
		double f = ((num12 > 0.30000001192092896) ? num12 : 0.30000001192092896);
		f = Maths::Levelize(f,0.7);
		num6 = ((num6 > -0.800000011920929) ? num6 : ((0.0 - f - num11) * 0.8999999761581421));
		num6 = ((num6 > -1.2000000476837158) ? num6 : (-1.2000000476837158));
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num6 + 0.2) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m128d warpedX = Levelize_batch2(_mm_mul_pd(x,_mm_set1_pd(0.007)));
		__m128d warpedY = Levelize_batch2(_mm_mul_pd(y,_mm_set1_pd(0.007)));
		__m128d warpedZ = Levelize_batch2(_mm_mul_pd(z,_mm_set1_pd(0.007)));
		__m128d fx = _mm_mul_pd(x,_mm_set1_pd(0.05));
		__m128d fy = _mm_mul_pd(y,_mm_set1_pd(0.05));
		__m128d fz = _mm_mul_pd(z,_mm_set1_pd(0.05));
		warpedX = _mm_add_pd(warpedX,_mm_mul_pd(simplexNoise.Noise_batch2(fx,fy,fz),_mm_set1_pd(0.04)));
		warpedY = _mm_add_pd(warpedY,_mm_mul_pd(simplexNoise.Noise_batch2(fy,fz,fx),_mm_set1_pd(0.04)));
		warpedZ = _mm_add_pd(warpedZ,_mm_mul_pd(simplexNoise.Noise_batch2(fz,fx,fy),_mm_set1_pd(0.04)));
		const __m128d zero = _mm_setzero_pd();
		const __m128d one = _mm_set1_pd(1.0);
		const __m128d sign = _mm_set1_pd(-0.0);
		__m128d noise = _mm_andnot_pd(sign,simplexNoise2.Noise_batch2(warpedX,warpedY,warpedZ));
		__m128d ridge = _mm_mul_pd(_mm_sub_pd(_mm_set1_pd(0.16),noise),_mm_set1_pd(10.0));
		ridge = _mm_and_pd(_mm_min_pd(ridge,one),_mm_cmp_pd(ridge,zero,_CMP_GT_OQ));
		ridge = _mm_mul_pd(ridge,ridge);
		__m128d envelope = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(y,_mm_set1_pd(0.005)),_mm_mul_pd(z,_mm_set1_pd(0.005)),_mm_mul_pd(x,_mm_set1_pd(0.005)),4);
		envelope = _mm_mul_pd(_mm_add_pd(envelope,_mm_set1_pd(0.22)),_mm_set1_pd(5.0));
		envelope = _mm_and_pd(_mm_min_pd(envelope,one),_mm_cmp_pd(envelope,zero,_CMP_GT_OQ));
		__m128d detail = _mm_andnot_pd(sign,simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(warpedX,_mm_set1_pd(1.5)),_mm_mul_pd(warpedY,_mm_set1_pd(1.5)),_mm_mul_pd(warpedZ,_mm_set1_pd(1.5)),2));
		__m128d height = _mm_mul_pd(_mm_mul_pd(ridge,_mm_set1_pd(-1.2)),envelope);
		__m128d raised = _mm_add_pd(height,_mm_add_pd(_mm_mul_pd(noise,_mm_set1_pd(0.25)),_mm_mul_pd(detail,_mm_set1_pd(0.6))));
		height = _mm_blendv_pd(height,raised,_mm_cmp_pd(height,zero,_CMP_GE_OQ));
		height = _mm_sub_pd(height,_mm_set1_pd(0.1));
		__m128d depth = _mm_sub_pd(_mm_set1_pd(-0.3),height);
		__m128d depressed = _mm_cmp_pd(depth,zero,_CMP_GT_OQ);

		depth = _mm_min_pd(depth,one);
		depth = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),depth),depth),depth),depth);
		__m128d lowered = _mm_sub_pd(_mm_set1_pd(-0.3),_mm_mul_pd(depth,_mm_set1_pd(3.700000047683716)));
		height = _mm_blendv_pd(height,lowered,depressed);
		__m128d floor = Levelize_batch2(_mm_max_pd(ridge,_mm_set1_pd(0.30000001192092896)),0.7);
		__m128d bottom = _mm_mul_pd(_mm_sub_pd(_mm_sub_pd(zero,floor),noise),_mm_set1_pd(0.8999999761581421));
		height = _mm_blendv_pd(bottom,height,_mm_cmp_pd(height,_mm_set1_pd(-0.800000011920929),_CMP_GT_OQ));
		height = _mm_max_pd(height,_mm_set1_pd(-1.2000000476837158));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.2)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Preserve float coordinate multiplication before widening to double.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));

		__m256d warpedX = Levelize_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.007)));
		__m256d warpedY = Levelize_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.007)));
		__m256d warpedZ = Levelize_batch4(_mm256_mul_pd(z,_mm256_set1_pd(0.007)));
		__m256d fx = _mm256_mul_pd(x,_mm256_set1_pd(0.05));
		__m256d fy = _mm256_mul_pd(y,_mm256_set1_pd(0.05));
		__m256d fz = _mm256_mul_pd(z,_mm256_set1_pd(0.05));
		warpedX = _mm256_add_pd(warpedX,_mm256_mul_pd(simplexNoise.Noise_batch4(fx,fy,fz),_mm256_set1_pd(0.04)));
		warpedY = _mm256_add_pd(warpedY,_mm256_mul_pd(simplexNoise.Noise_batch4(fy,fz,fx),_mm256_set1_pd(0.04)));
		warpedZ = _mm256_add_pd(warpedZ,_mm256_mul_pd(simplexNoise.Noise_batch4(fz,fx,fy),_mm256_set1_pd(0.04)));
		const __m256d zero = _mm256_setzero_pd();
		const __m256d one = _mm256_set1_pd(1.0);
		const __m256d sign = _mm256_set1_pd(-0.0);
		__m256d noise = _mm256_andnot_pd(sign,simplexNoise2.Noise_batch4(warpedX,warpedY,warpedZ));
		__m256d ridge = _mm256_mul_pd(_mm256_sub_pd(_mm256_set1_pd(0.16),noise),_mm256_set1_pd(10.0));
		ridge = _mm256_and_pd(_mm256_min_pd(ridge,one),_mm256_cmp_pd(ridge,zero,_CMP_GT_OQ));
		ridge = _mm256_mul_pd(ridge,ridge);
		__m256d envelope = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.005)),_mm256_mul_pd(z,_mm256_set1_pd(0.005)),_mm256_mul_pd(x,_mm256_set1_pd(0.005)),4);
		envelope = _mm256_mul_pd(_mm256_add_pd(envelope,_mm256_set1_pd(0.22)),_mm256_set1_pd(5.0));
		envelope = _mm256_and_pd(_mm256_min_pd(envelope,one),_mm256_cmp_pd(envelope,zero,_CMP_GT_OQ));
		__m256d detail = _mm256_andnot_pd(sign,simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(warpedX,_mm256_set1_pd(1.5)),_mm256_mul_pd(warpedY,_mm256_set1_pd(1.5)),_mm256_mul_pd(warpedZ,_mm256_set1_pd(1.5)),2));
		__m256d height = _mm256_mul_pd(_mm256_mul_pd(ridge,_mm256_set1_pd(-1.2)),envelope);
		__m256d raised = _mm256_add_pd(height,_mm256_add_pd(_mm256_mul_pd(noise,_mm256_set1_pd(0.25)),_mm256_mul_pd(detail,_mm256_set1_pd(0.6))));
		height = _mm256_blendv_pd(height,raised,_mm256_cmp_pd(height,zero,_CMP_GE_OQ));
		height = _mm256_sub_pd(height,_mm256_set1_pd(0.1));
		__m256d depth = _mm256_sub_pd(_mm256_set1_pd(-0.3),height);
		__m256d depressed = _mm256_cmp_pd(depth,zero,_CMP_GT_OQ);

		depth = _mm256_min_pd(depth,one);
		depth = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),depth),depth),depth),depth);
		__m256d lowered = _mm256_sub_pd(_mm256_set1_pd(-0.3),_mm256_mul_pd(depth,_mm256_set1_pd(3.700000047683716)));
		height = _mm256_blendv_pd(height,lowered,depressed);
		__m256d floor = Levelize_batch4(_mm256_max_pd(ridge,_mm256_set1_pd(0.30000001192092896)),0.7);
		__m256d bottom = _mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(zero,floor),noise),_mm256_set1_pd(0.8999999761581421));
		height = _mm256_blendv_pd(bottom,height,_mm256_cmp_pd(height,_mm256_set1_pd(-0.800000011920929),_CMP_GT_OQ));
		height = _mm256_max_pd(height,_mm256_set1_pd(-1.2000000476837158));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.2)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num = dotNet35Random.Next();
		int num2 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num);
		simplexNoise2 = SimplexNoise(num2);
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain6");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,tls.buffer);
			kernel.setArg(3,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm7: public PlanetAlgorithm
{
public:
	static constexpr double num = 0.008;
	static constexpr double num2 = 0.01;
	static constexpr double num3 = 0.01;
	static constexpr double num4 = 3.0;
	static constexpr double num5 = -2.4;
	static constexpr double num6 = 0.9;
	static constexpr double num7 = 0.5;
	static constexpr double num8 = 2.5;
	static constexpr double num9 = 0.3;

	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num12 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num13 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num14 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num17 = simplexNoise.Noise3DFBM(num12 * num,num13 * num2,num14 * num3,6) * num4 + num5;
		double num18 = simplexNoise2.Noise3DFBM(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3) * num4 * num6 + num7;
		double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
		double num20 = num17 + num19;
		double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
		double num22 = ((num21 > 0.0) ? Maths::Levelize3(num21,0.7) : Maths::Levelize2(num21,0.5));
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num22) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply in float before converting, matching GenerateHeight1.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d terrain = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_mul_pd(z,_mm_set1_pd(num3)),6);
		terrain = _mm_add_pd(_mm_mul_pd(terrain,_mm_set1_pd(num4)),_mm_set1_pd(num5));
		__m128d detail = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(0.0025)),_mm_mul_pd(y,_mm_set1_pd(0.0025)),_mm_mul_pd(z,_mm_set1_pd(0.0025)),3);
		detail = _mm_add_pd(_mm_mul_pd(_mm_mul_pd(detail,_mm_set1_pd(num4)),_mm_set1_pd(num6)),_mm_set1_pd(num7));
		const __m128d zero = _mm_setzero_pd();
		detail = _mm_blendv_pd(detail,_mm_mul_pd(detail,_mm_set1_pd(0.5)),_mm_cmp_pd(detail,zero,_CMP_GT_OQ));
		__m128d height = _mm_add_pd(terrain,detail);
		height = _mm_mul_pd(height,_mm_blendv_pd(_mm_set1_pd(1.6),_mm_set1_pd(0.5),_mm_cmp_pd(height,zero,_CMP_GT_OQ)));
		const __m128d positive = _mm_cmp_pd(height,zero,_CMP_GT_OQ);
		const __m128d level = _mm_blendv_pd(_mm_set1_pd(0.5),_mm_set1_pd(0.7),positive);
		__m128d scaled = _mm_div_pd(height,level);
		__m128d integral = _mm_floor_pd(scaled);
		__m128d fraction = _mm_sub_pd(scaled,integral);
		// Levelize2 for every lane; positive heights receive a third smoothing step.
		for(int step=0;step<2;step++)
			fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		__m128d third = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		fraction = _mm_blendv_pd(fraction,third,positive);
		height = _mm_mul_pd(_mm_add_pd(integral,fraction),level);
		height = _mm_mul_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply in float before converting, matching GenerateHeight1.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d terrain = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_mul_pd(z,_mm256_set1_pd(num3)),6);
		terrain = _mm256_add_pd(_mm256_mul_pd(terrain,_mm256_set1_pd(num4)),_mm256_set1_pd(num5));
		__m256d detail = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.0025)),_mm256_mul_pd(y,_mm256_set1_pd(0.0025)),_mm256_mul_pd(z,_mm256_set1_pd(0.0025)),3);
		detail = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(detail,_mm256_set1_pd(num4)),_mm256_set1_pd(num6)),_mm256_set1_pd(num7));
		const __m256d zero = _mm256_setzero_pd();
		detail = _mm256_blendv_pd(detail,_mm256_mul_pd(detail,_mm256_set1_pd(0.5)),_mm256_cmp_pd(detail,zero,_CMP_GT_OQ));
		__m256d height = _mm256_add_pd(terrain,detail);
		height = _mm256_mul_pd(height,_mm256_blendv_pd(_mm256_set1_pd(1.6),_mm256_set1_pd(0.5),_mm256_cmp_pd(height,zero,_CMP_GT_OQ)));
		const __m256d positive = _mm256_cmp_pd(height,zero,_CMP_GT_OQ);
		const __m256d level = _mm256_blendv_pd(_mm256_set1_pd(0.5),_mm256_set1_pd(0.7),positive);
		__m256d scaled = _mm256_div_pd(height,level);
		__m256d integral = _mm256_floor_pd(scaled);
		__m256d fraction = _mm256_sub_pd(scaled,integral);
		// Levelize2 for every lane; positive heights receive a third smoothing step.
		for(int step=0;step<2;step++)
			fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		__m256d third = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		fraction = _mm256_blendv_pd(fraction,third,positive);
		height = _mm256_mul_pd(_mm256_add_pd(integral,fraction),level);
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num10 = dotNet35Random.Next();
		int num11 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num10);
		simplexNoise2 = SimplexNoise(num11);
		heightData.assign(VERTICES_DATALENGTH,0);
		//水世界不需要生成地形
		//if(gen_terr && OpenCLManager::get_worker()) {
		//	cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain7");

		//	ThreadLocalBuffers& tls = get_tls_buffers();
		//	memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
		//	memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
		//	memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
		//	memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
		//	tls.upload_buffer();

		//	kernel.setArg(0,OpenCLManager::vertices_buffer);
		//	kernel.setArg(1,planet.radius);
		//	kernel.setArg(2,tls.buffer);
		//	kernel.setArg(3,tls.heightData_buffer);

		//	int local_size = OpenCLManager::local_size;
		//	int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
		//	cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
		//	if(err != CL_SUCCESS){
		//		std::cerr << "Kernel execution failed with error code: " << err << std::endl;
		//		throw std::runtime_error("Kernel execution failed");
		//	}

		//	tls.download_buffer(heightData.data());
		//	OpenCLManager::return_worker();
		//	for(int i=LAND_DATALENGTH-1;i>=0;i--) {
		//		heightData[landIndex[i]] = heightData[i];
		//		heightData[i] = 0;
		//	}
		//}
	}

	bool check_vein_position(const Vector3& target_pos,EVeinType vein_type) override {
		return vein_type == EVeinType::Bamboo && QueryHeight(target_pos) > NORMAL_PLANET_REAL_RADIUS - 4.0f;
	}
};

class PlanetAlgorithm8: public PlanetAlgorithm
{
private:
#ifdef SUPPORT_AVX2
	__forceinline __m128 __vectorcall CosPower_batch2(__m128 angle) {
		// Preserve Mathf.Cos/Pow rounding; two-point batches use the lower two lanes.
		float lanes[4] = {};
		_mm_storeu_ps(lanes,angle);
		for(int lane=0;lane<2;lane++) {
			float f = Mathf.Cos(lanes[lane]) * 1.1f;
			lanes[lane] = Mathf.Sign(f) * Mathf.Pow(f,4.0f);
		}
		return _mm_loadu_ps(lanes);
	}
	__forceinline __m128 __vectorcall CosPower_batch4(__m128 angle) {
		// Preserve Mathf.Cos/Pow rounding; two-point batches use the lower two lanes.
		float lanes[4] = {};
		_mm_storeu_ps(lanes,angle);
		for(int lane=0;lane<4;lane++) {
			float f = Mathf.Cos(lanes[lane]) * 1.1f;
			lanes[lane] = Mathf.Sign(f) * Mathf.Pow(f,4.0f);
		}
		return _mm_loadu_ps(lanes);
	}
#endif
public:
	double num,num2,num3,modY;
	SimplexNoise simplexNoise;

	void GenerateHeight1(int index) override {
		double num4 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num5 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num6 = vertices[index].z * NORMAL_PLANET_RADIUS;
		float num9 = Mathf.Clamp((float)simplexNoise.Noise3DFBM(num4 * num,num5 * num2,num6 * num3,6,0.45,1.8) + 1.0f + (float)modY * 0.01f,0.0f,2.0f);
		float num10 = 0.0f;
		if((double)num9 < 1.0) {
			float f = Mathf.Cos(num9 * MATHF_PI) * 1.1f;
			f = Mathf.Sign(f) * Mathf.Pow(f,4.0f);
			f = Mathf.Clamp(f,-1.0f,1.0f);
			num10 = 1.0f - (f + 1.0f) * 0.5f;
		} else {
			float f2 = Mathf.Cos((num9 - 1.0f) * MATHF_PI) * 1.1f;
			f2 = Mathf.Sign(f2) * Mathf.Pow(f2,4.0f);
			f2 = Mathf.Clamp(f2,-1.0f,1.0f);
			num10 = 2.0f - (f2 + 1.0f) * 0.5f;
		}
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num10 + 0.1) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d noise = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_mul_pd(z,_mm_set1_pd(num3)),6,0.45,1.8);
		__m128 value = _mm_add_ps(_mm_add_ps(_mm_cvtpd_ps(noise),_mm_set1_ps(1.0f)),_mm_set1_ps((float)modY * 0.01f));
		value = _mm_min_ps(_mm_set1_ps(2.0f),_mm_max_ps(_mm_setzero_ps(),value));
		__m128 lower = _mm_cmp_ps(value,_mm_set1_ps(1.0f),_CMP_LT_OQ);
		__m128 phase = _mm_blendv_ps(_mm_sub_ps(value,_mm_set1_ps(1.0f)),value,lower);
		__m128 curve = CosPower_batch2(_mm_mul_ps(phase,_mm_set1_ps(MATHF_PI)));
		curve = _mm_min_ps(_mm_set1_ps(1.0f),_mm_max_ps(_mm_set1_ps(-1.0f),curve));
		__m128 offset = _mm_blendv_ps(_mm_set1_ps(2.0f),_mm_set1_ps(1.0f),lower);
		__m128 shaped = _mm_sub_ps(offset,_mm_mul_ps(_mm_add_ps(curve,_mm_set1_ps(1.0f)),_mm_set1_ps(0.5f)));
		__m128d height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),_mm_cvtps_pd(shaped)),_mm_set1_pd(0.1)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d noise = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_mul_pd(z,_mm256_set1_pd(num3)),6,0.45,1.8);
		__m128 value = _mm_add_ps(_mm_add_ps(_mm256_cvtpd_ps(noise),_mm_set1_ps(1.0f)),_mm_set1_ps((float)modY * 0.01f));
		value = _mm_min_ps(_mm_set1_ps(2.0f),_mm_max_ps(_mm_setzero_ps(),value));
		__m128 lower = _mm_cmp_ps(value,_mm_set1_ps(1.0f),_CMP_LT_OQ);
		__m128 phase = _mm_blendv_ps(_mm_sub_ps(value,_mm_set1_ps(1.0f)),value,lower);
		__m128 curve = CosPower_batch4(_mm_mul_ps(phase,_mm_set1_ps(MATHF_PI)));
		curve = _mm_min_ps(_mm_set1_ps(1.0f),_mm_max_ps(_mm_set1_ps(-1.0f),curve));
		__m128 offset = _mm_blendv_ps(_mm_set1_ps(2.0f),_mm_set1_ps(1.0f),lower);
		__m128 shaped = _mm_sub_ps(offset,_mm_mul_ps(_mm_add_ps(curve,_mm_set1_ps(1.0f)),_mm_set1_ps(0.5f)));
		__m256d height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),_mm256_cvtps_pd(shaped)),_mm256_set1_pd(0.1)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		double modX = planet.mod_x;
		modY = planet.mod_y;
		num = 0.002 * modX;
		num2 = 0.002 * modX * modX * 6.66667;
		num3 = 0.002 * modX;
		simplexNoise = SimplexNoise(DotNet35Random(planet.seed).Next());
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain8");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,num);
			kernel.setArg(3,num2);
			kernel.setArg(4,num3);
			kernel.setArg(5,modY);
			kernel.setArg(6,tls.buffer);
			kernel.setArg(7,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm9: public PlanetAlgorithm
{
private:
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Pow_batch2(__m128d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[2];
		_mm_storeu_pd(lanes,value);
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm_loadu_pd(lanes);
	}
	__forceinline __m256d __vectorcall Pow_batch4(__m256d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[4];
		_mm256_storeu_pd(lanes,value);
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm256_loadu_pd(lanes);
	}
#endif
public:
	static constexpr double num = 0.01;
	static constexpr double num2 = 0.012;
	static constexpr double num3 = 0.01;
	static constexpr double num4 = 3.0;
	static constexpr double num5 = -0.2;
	static constexpr double num6 = 0.9;
	static constexpr double num7 = 0.5;
	static constexpr double num8 = 2.5;
	static constexpr double num9 = 0.3;

	double modX,modY;
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num12 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num13 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num14 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num17 = simplexNoise.Noise3DFBM(num12 * num * 0.75,num13 * num2 * 0.5,num14 * num3 * 0.75,6) * num4 + num5;
		double num18 = simplexNoise2.Noise3DFBM(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3) * num4 * num6 + num7;
		double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
		double num20 = num17 + num19;
		double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
		double num22 = ((num21 > 0.0) ? Maths::Levelize3(num21,0.7) : Maths::Levelize2(num21,0.5));
		num22 += 0.618;
		num22 = ((num22 > -1.0) ? (num22 * 1.5) : (num22 * 4.0));
		double num23 = simplexNoise2.Noise3DFBM(num12 * num * 2.5,num13 * num2 * 8.0,num14 * num3 * 2.5,2) * 0.6 - 0.3;
		double num24 = num21 * num8 + num23 + num9;
		double val = Maths::Levelize(num21 + 0.7);
		double num25 = simplexNoise.Noise3DFBM(num12 * num * modX,num13 * num2 * modX,num14 * num3 * modX,6) * num4 + num5;
		double num26 = simplexNoise2.Noise3DFBM(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3) * num4 * num6 + num7;
		double num27 = ((num26 > 0.0) ? (num26 * 0.5) : num26);
		double x = (num25 + num27 + 5.0) * 0.13;
		x = Math.Pow(x,6.0) * 24.0 - 24.0;
		double num28 = ((num22 >= 0.0 - modY) ? 0.0 : Math.Pow(Math.Min(Math.Abs(num22 + modY) / 5.0,1.0),1.0));
		double num15 = num22 * (1.0 - num28) + x * num28;
		num15 = ((num15 > 0.0) ? (num15 * 0.5) : num15);
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num15 + 0.2) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply in float before converting, matching GenerateHeight1.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d terrain = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_set1_pd(0.75)),_mm_mul_pd(_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_set1_pd(0.5)),_mm_mul_pd(_mm_mul_pd(z,_mm_set1_pd(num3)),_mm_set1_pd(0.75)),6);
		terrain = _mm_add_pd(_mm_mul_pd(terrain,_mm_set1_pd(num4)),_mm_set1_pd(num5));
		__m128d detail = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(0.0025)),_mm_mul_pd(y,_mm_set1_pd(0.0025)),_mm_mul_pd(z,_mm_set1_pd(0.0025)),3);
		detail = _mm_add_pd(_mm_mul_pd(_mm_mul_pd(detail,_mm_set1_pd(num4)),_mm_set1_pd(num6)),_mm_set1_pd(num7));
		const __m128d zero = _mm_setzero_pd();
		detail = _mm_blendv_pd(detail,_mm_mul_pd(detail,_mm_set1_pd(0.5)),_mm_cmp_pd(detail,zero,_CMP_GT_OQ));
		__m128d height = _mm_add_pd(terrain,detail);
		height = _mm_mul_pd(height,_mm_blendv_pd(_mm_set1_pd(1.6),_mm_set1_pd(0.5),_mm_cmp_pd(height,zero,_CMP_GT_OQ)));
		const __m128d positive = _mm_cmp_pd(height,zero,_CMP_GT_OQ);
		const __m128d level = _mm_blendv_pd(_mm_set1_pd(0.5),_mm_set1_pd(0.7),positive);
		__m128d scaled = _mm_div_pd(height,level);
		__m128d integral = _mm_floor_pd(scaled);
		__m128d fraction = _mm_sub_pd(scaled,integral);
		// Levelize2 for every lane; positive heights receive a third smoothing step.
		for(int step=0;step<2;step++)
			fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		__m128d third = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		fraction = _mm_blendv_pd(fraction,third,positive);
		height = _mm_mul_pd(_mm_add_pd(integral,fraction),level);

		height = _mm_add_pd(height,_mm_set1_pd(0.618));
		height = _mm_mul_pd(height,_mm_blendv_pd(_mm_set1_pd(4.0),_mm_set1_pd(1.5),_mm_cmp_pd(height,_mm_set1_pd(-1.0),_CMP_GT_OQ)));
		__m128d deep = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_set1_pd(modX)),_mm_mul_pd(_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_set1_pd(modX)),_mm_mul_pd(_mm_mul_pd(z,_mm_set1_pd(num3)),_mm_set1_pd(modX)),6);
		deep = _mm_add_pd(_mm_mul_pd(deep,_mm_set1_pd(num4)),_mm_set1_pd(num5));
		// detail already contains the identical low-frequency noise and positive-lane scaling.
		deep = _mm_mul_pd(_mm_add_pd(_mm_add_pd(deep,detail),_mm_set1_pd(5.0)),_mm_set1_pd(0.13));
		deep = _mm_sub_pd(_mm_mul_pd(Pow_batch2(deep,6.0),_mm_set1_pd(24.0)),_mm_set1_pd(24.0));
		__m128d weight = _mm_min_pd(_mm_div_pd(_mm_andnot_pd(_mm_set1_pd(-0.0),_mm_add_pd(height,_mm_set1_pd(modY))),_mm_set1_pd(5.0)),_mm_set1_pd(1.0));
		weight = Pow_batch2(weight,1.0);
		weight = _mm_blendv_pd(weight,zero,_mm_cmp_pd(height,_mm_set1_pd(0.0-modY),_CMP_GE_OQ));
		height = _mm_add_pd(_mm_mul_pd(height,_mm_sub_pd(_mm_set1_pd(1.0),weight)),_mm_mul_pd(deep,weight));
		height = _mm_blendv_pd(height,_mm_mul_pd(height,_mm_set1_pd(0.5)),_mm_cmp_pd(height,zero,_CMP_GT_OQ));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.2)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply in float before converting, matching GenerateHeight1.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d terrain = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_set1_pd(0.75)),_mm256_mul_pd(_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_set1_pd(0.5)),_mm256_mul_pd(_mm256_mul_pd(z,_mm256_set1_pd(num3)),_mm256_set1_pd(0.75)),6);
		terrain = _mm256_add_pd(_mm256_mul_pd(terrain,_mm256_set1_pd(num4)),_mm256_set1_pd(num5));
		__m256d detail = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.0025)),_mm256_mul_pd(y,_mm256_set1_pd(0.0025)),_mm256_mul_pd(z,_mm256_set1_pd(0.0025)),3);
		detail = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(detail,_mm256_set1_pd(num4)),_mm256_set1_pd(num6)),_mm256_set1_pd(num7));
		const __m256d zero = _mm256_setzero_pd();
		detail = _mm256_blendv_pd(detail,_mm256_mul_pd(detail,_mm256_set1_pd(0.5)),_mm256_cmp_pd(detail,zero,_CMP_GT_OQ));
		__m256d height = _mm256_add_pd(terrain,detail);
		height = _mm256_mul_pd(height,_mm256_blendv_pd(_mm256_set1_pd(1.6),_mm256_set1_pd(0.5),_mm256_cmp_pd(height,zero,_CMP_GT_OQ)));
		const __m256d positive = _mm256_cmp_pd(height,zero,_CMP_GT_OQ);
		const __m256d level = _mm256_blendv_pd(_mm256_set1_pd(0.5),_mm256_set1_pd(0.7),positive);
		__m256d scaled = _mm256_div_pd(height,level);
		__m256d integral = _mm256_floor_pd(scaled);
		__m256d fraction = _mm256_sub_pd(scaled,integral);
		// Levelize2 for every lane; positive heights receive a third smoothing step.
		for(int step=0;step<2;step++)
			fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		__m256d third = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		fraction = _mm256_blendv_pd(fraction,third,positive);
		height = _mm256_mul_pd(_mm256_add_pd(integral,fraction),level);

		height = _mm256_add_pd(height,_mm256_set1_pd(0.618));
		height = _mm256_mul_pd(height,_mm256_blendv_pd(_mm256_set1_pd(4.0),_mm256_set1_pd(1.5),_mm256_cmp_pd(height,_mm256_set1_pd(-1.0),_CMP_GT_OQ)));
		__m256d deep = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_set1_pd(modX)),_mm256_mul_pd(_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_set1_pd(modX)),_mm256_mul_pd(_mm256_mul_pd(z,_mm256_set1_pd(num3)),_mm256_set1_pd(modX)),6);
		deep = _mm256_add_pd(_mm256_mul_pd(deep,_mm256_set1_pd(num4)),_mm256_set1_pd(num5));
		// detail already contains the identical low-frequency noise and positive-lane scaling.
		deep = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(deep,detail),_mm256_set1_pd(5.0)),_mm256_set1_pd(0.13));
		deep = _mm256_sub_pd(_mm256_mul_pd(Pow_batch4(deep,6.0),_mm256_set1_pd(24.0)),_mm256_set1_pd(24.0));
		__m256d weight = _mm256_min_pd(_mm256_div_pd(_mm256_andnot_pd(_mm256_set1_pd(-0.0),_mm256_add_pd(height,_mm256_set1_pd(modY))),_mm256_set1_pd(5.0)),_mm256_set1_pd(1.0));
		weight = Pow_batch4(weight,1.0);
		weight = _mm256_blendv_pd(weight,zero,_mm256_cmp_pd(height,_mm256_set1_pd(0.0-modY),_CMP_GE_OQ));
		height = _mm256_add_pd(_mm256_mul_pd(height,_mm256_sub_pd(_mm256_set1_pd(1.0),weight)),_mm256_mul_pd(deep,weight));
		height = _mm256_blendv_pd(height,_mm256_mul_pd(height,_mm256_set1_pd(0.5)),_mm256_cmp_pd(height,zero,_CMP_GT_OQ));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.2)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		modX = planet.mod_x;
		modY = planet.mod_y;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num10 = dotNet35Random.Next();
		int num11 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num10);
		simplexNoise2 = SimplexNoise(num11);
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain9");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,modX);
			kernel.setArg(3,modY);
			kernel.setArg(4,tls.buffer);
			kernel.setArg(5,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm10: public PlanetAlgorithm
{
private:
	double Max(double a,double b)
	{
		if((a > b))
		{
			return a;
		}
		return b;
	}
	double Remap(double sourceMin,double sourceMax,double targetMin,double targetMax,double x)
	{
		return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
	}
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Levelize_batch2(__m128d value,int steps = 1,double level = 1.0) {
		value = _mm_div_pd(value,_mm_set1_pd(level));
		__m128d integral = _mm_floor_pd(value);
		__m128d fraction = _mm_sub_pd(value,integral);
		for(int step=0;step<steps;step++)
			fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm_mul_pd(_mm_add_pd(integral,fraction),_mm_set1_pd(level));
	}
	__forceinline __m128d __vectorcall Remap_batch2(double sourceMin,double sourceMax,double targetMin,double targetMax,__m128d x) {
		return _mm_add_pd(_mm_mul_pd(_mm_div_pd(_mm_sub_pd(x,_mm_set1_pd(sourceMin)),_mm_set1_pd(sourceMax-sourceMin)),_mm_set1_pd(targetMax-targetMin)),_mm_set1_pd(targetMin));
	}
	__forceinline __m128d __vectorcall Max_batch2(__m128d a,__m128d b) {
		return _mm_max_pd(a,b);
	}
	__forceinline __m128d __vectorcall Sin_batch2(__m128d value) {
		double lanes[2];
		_mm_storeu_pd(lanes,value);
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Math.Sin(lanes[lane]);
		return _mm_loadu_pd(lanes);
	}
	__forceinline __m128d __vectorcall PowFloat_batch2(__m128d value,float exponent) {
		// Match the scalar float cast and Mathf.Pow before widening back to double.
		float lanes[4];
		_mm_storeu_ps(lanes,_mm_cvtpd_ps(value));
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Mathf.Pow(lanes[lane],exponent);
		return _mm_cvtps_pd(_mm_loadu_ps(lanes));
	}
	__forceinline __m256d __vectorcall Levelize_batch4(__m256d value,int steps = 1,double level = 1.0) {
		value = _mm256_div_pd(value,_mm256_set1_pd(level));
		__m256d integral = _mm256_floor_pd(value);
		__m256d fraction = _mm256_sub_pd(value,integral);
		for(int step=0;step<steps;step++)
			fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm256_mul_pd(_mm256_add_pd(integral,fraction),_mm256_set1_pd(level));
	}
	__forceinline __m256d __vectorcall Remap_batch4(double sourceMin,double sourceMax,double targetMin,double targetMax,__m256d x) {
		return _mm256_add_pd(_mm256_mul_pd(_mm256_div_pd(_mm256_sub_pd(x,_mm256_set1_pd(sourceMin)),_mm256_set1_pd(sourceMax-sourceMin)),_mm256_set1_pd(targetMax-targetMin)),_mm256_set1_pd(targetMin));
	}
	__forceinline __m256d __vectorcall Max_batch4(__m256d a,__m256d b) {
		return _mm256_max_pd(a,b);
	}
	__forceinline __m256d __vectorcall Sin_batch4(__m256d value) {
		double lanes[4];
		_mm256_storeu_pd(lanes,value);
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Math.Sin(lanes[lane]);
		return _mm256_loadu_pd(lanes);
	}
	__forceinline __m256d __vectorcall PowFloat_batch4(__m256d value,float exponent) {
		// Match the scalar float cast and Mathf.Pow before widening back to double.
		float lanes[4];
		_mm_storeu_ps(lanes,_mm256_cvtpd_ps(value));
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Mathf.Pow(lanes[lane],exponent);
		return _mm256_cvtps_pd(_mm_loadu_ps(lanes));
	}
#endif
public:
	static constexpr int kCircleCount = 10;
	static constexpr double num = 0.007;
	static constexpr double num2 = 0.007;
	static constexpr double num3 = 0.007;

	Vector4 ellipses[10];
	double eccentricities[10];
	double heights[10];
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;
	SimplexNoise simplexNoise3;
	SimplexNoise simplexNoise4;

	void GenerateHeight1(int index) override {
		double num9 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num10 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num11 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num12 = Maths::Levelize(num9 * 0.007);
		double num13 = Maths::Levelize(num10 * 0.007);
		double num14 = Maths::Levelize(num11 * 0.007);
		num12 += simplexNoise3.Noise(num9 * 0.05,num10 * 0.05,num11 * 0.05) * 0.04;
		num13 += simplexNoise3.Noise(num10 * 0.05,num11 * 0.05,num9 * 0.05) * 0.04;
		num14 += simplexNoise3.Noise(num11 * 0.05,num9 * 0.05,num10 * 0.05) * 0.04;
		double num15 = Math.Abs(simplexNoise4.Noise(num12,num13,num14));
		double num16 = (0.16 - num15) * 10.0;
		num16 = ((!(num16 > 0.0)) ? 0.0 : ((num16 > 1.0) ? 1.0 : num16));
		num16 *= num16;
		double num17 = (simplexNoise3.Noise3DFBM(num10 * 0.005,num11 * 0.005,num9 * 0.005,4) + 0.22) * 5.0;
		num17 = ((!(num17 > 0.0)) ? 0.0 : ((num17 > 1.0) ? 1.0 : num17));
		double num18 = Math.Abs(simplexNoise4.Noise3DFBM(num12 * 1.5,num13 * 1.5,num14 * 1.5,2));
		double num19 = 0.0;
		double num20 = 0.0;
		double num21 = simplexNoise2.Noise3DFBM(num9 * num * 5.0,num10 * num2 * 5.0,num11 * num3 * 5.0,4);
		double num22 = num21 * 0.2;
		double num23 = 0.0;
		for(int k = 0; k < 10; k++)
		{
			double num24 = (double)ellipses[k].x - num9;
			double num25 = (double)ellipses[k].y - num10;
			double num26 = (double)ellipses[k].z - num11;
			double num27 = eccentricities[k] * num24 * num24 + num25 * num25 + num26 * num26;
			num27 = Remap(-1.0,1.0,0.2,5.0,num21) * num27;
			if(!(num27 >= (double)(ellipses[k].w * ellipses[k].w)))
			{
				double num28 = 1.0f - Mathf.Sqrt((float)(num27 / (double)(ellipses[k].w * ellipses[k].w)));
				double num29 = 1.0 - num28;
				double num30 = 1.0 - num29 * num29 * num29 * num29 + num22 * 2.0;
				if(num30 < 0.0)
				{
					num30 = 0.0;
				}
				num23 = Max(num23,heights[k] * num30);
			}
		}
		num9 += Math.Sin(num10 * 0.15) * 2.0;
		num10 += Math.Sin(num11 * 0.15) * 2.0;
		num11 += Math.Sin(num9 * 0.15) * 2.0;
		num9 *= num;
		num10 *= num2;
		num11 *= num3;
		double f = Mathf.Pow((float)((simplexNoise.Noise3DFBM(num9 * 0.6,num10 * 0.6,num11 * 0.6,4,0.5,1.8) + 1.0) * 0.5),1.3f);
		double x = simplexNoise2.Noise3DFBM(num9 * 6.0,num10 * 6.0,num11 * 6.0,5);
		x = Remap(-1.0,1.0,-0.1,0.15,x);
		double num31 = simplexNoise2.Noise3DFBM(num9 * 5.0 * 3.0,num10 * 5.0,num11 * 5.0,1);
		double num32 = simplexNoise2.Noise3DFBM(num9 * 5.0 * 3.0 + num31 * 0.3,num10 * 5.0 + num31 * 0.3,num11 * 5.0 + num31 * 0.3,5) * 0.1;
		f = (float)Maths::Levelize(Maths::Levelize4(f));
		f = Math.Min(1.0,f);
		if(!(f > 0.8))
			f = ((!(f > 0.4)) ? (f + x) : (f + num32));
		double a = f * 2.5 - f * num23;
		num19 = Max(a,x * 2.0);
		double num33 = (2.0 - num19) / 2.0;
		num19 -= num16 * 1.2 * num17 * num33;
		if(num19 >= 0.0)
			num19 += (num15 * 0.25 + num18 * 0.6) * num33;
		num19 -= 0.1;
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num19 + 0.1) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d warpedX = Levelize_batch2(_mm_mul_pd(x,_mm_set1_pd(0.007)));
		__m128d warpedY = Levelize_batch2(_mm_mul_pd(y,_mm_set1_pd(0.007)));
		__m128d warpedZ = Levelize_batch2(_mm_mul_pd(z,_mm_set1_pd(0.007)));
		__m128d fx = _mm_mul_pd(x,_mm_set1_pd(0.05));
		__m128d fy = _mm_mul_pd(y,_mm_set1_pd(0.05));
		__m128d fz = _mm_mul_pd(z,_mm_set1_pd(0.05));
		warpedX = _mm_add_pd(warpedX,_mm_mul_pd(simplexNoise3.Noise_batch2(fx,fy,fz),_mm_set1_pd(0.04)));
		warpedY = _mm_add_pd(warpedY,_mm_mul_pd(simplexNoise3.Noise_batch2(fy,fz,fx),_mm_set1_pd(0.04)));
		warpedZ = _mm_add_pd(warpedZ,_mm_mul_pd(simplexNoise3.Noise_batch2(fz,fx,fy),_mm_set1_pd(0.04)));
		const __m128d zero = _mm_setzero_pd();
		const __m128d one = _mm_set1_pd(1.0);
		const __m128d sign = _mm_set1_pd(-0.0);
		__m128d noise = _mm_andnot_pd(sign,simplexNoise4.Noise_batch2(warpedX,warpedY,warpedZ));
		__m128d ridge = _mm_mul_pd(_mm_sub_pd(_mm_set1_pd(0.16),noise),_mm_set1_pd(10.0));
		ridge = _mm_and_pd(_mm_min_pd(ridge,one),_mm_cmp_pd(ridge,zero,_CMP_GT_OQ));
		ridge = _mm_mul_pd(ridge,ridge);
		__m128d envelope = simplexNoise3.Noise3DFBM_batch2(_mm_mul_pd(y,_mm_set1_pd(0.005)),_mm_mul_pd(z,_mm_set1_pd(0.005)),_mm_mul_pd(x,_mm_set1_pd(0.005)),4);
		envelope = _mm_mul_pd(_mm_add_pd(envelope,_mm_set1_pd(0.22)),_mm_set1_pd(5.0));
		envelope = _mm_and_pd(_mm_min_pd(envelope,one),_mm_cmp_pd(envelope,zero,_CMP_GT_OQ));
		__m128d detail = _mm_andnot_pd(sign,simplexNoise4.Noise3DFBM_batch2(_mm_mul_pd(warpedX,_mm_set1_pd(1.5)),_mm_mul_pd(warpedY,_mm_set1_pd(1.5)),_mm_mul_pd(warpedZ,_mm_set1_pd(1.5)),2));

		__m128d craterNoise = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_set1_pd(5.0)),_mm_mul_pd(_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_set1_pd(5.0)),_mm_mul_pd(_mm_mul_pd(z,_mm_set1_pd(num3)),_mm_set1_pd(5.0)),4);
		__m128d craterDetail = _mm_mul_pd(craterNoise,_mm_set1_pd(0.2));
		__m128d distanceScale = Remap_batch2(-1.0,1.0,0.2,5.0,craterNoise);
		__m128d peak = zero;
		for(int k=0;k<kCircleCount;k++) {
			__m128d dx = _mm_sub_pd(_mm_set1_pd((double)ellipses[k].x),x);
			__m128d dy = _mm_sub_pd(_mm_set1_pd((double)ellipses[k].y),y);
			__m128d dz = _mm_sub_pd(_mm_set1_pd((double)ellipses[k].z),z);
			__m128d distance = _mm_add_pd(_mm_add_pd(_mm_mul_pd(_mm_mul_pd(_mm_set1_pd(eccentricities[k]),dx),dx),_mm_mul_pd(dy,dy)),_mm_mul_pd(dz,dz));
			distance = _mm_mul_pd(distanceScale,distance);
			// The radius square and square root are evaluated in float in the scalar path.
			__m128d width = _mm_set1_pd((double)(ellipses[k].w * ellipses[k].w));
			__m128d inside = _mm_cmp_pd(distance,width,_CMP_NGE_UQ);
			if(_mm_movemask_pd(inside) == 0)
				continue;
			__m128d falloff = _mm_cvtps_pd(_mm_sub_ps(_mm_set1_ps(1.0f),_mm_sqrt_ps(_mm_cvtpd_ps(_mm_div_pd(distance,width)))));
			__m128d t = _mm_sub_pd(one,falloff);
			__m128d fourth = _mm_mul_pd(_mm_mul_pd(_mm_mul_pd(t,t),t),t);
			__m128d shape = _mm_add_pd(_mm_sub_pd(one,fourth),_mm_mul_pd(craterDetail,_mm_set1_pd(2.0)));
			shape = _mm_max_pd(zero,shape);
			peak = _mm_blendv_pd(peak,Max_batch2(peak,_mm_mul_pd(_mm_set1_pd(heights[k]),shape)),inside);
		}
		x = _mm_add_pd(x,_mm_mul_pd(Sin_batch2(_mm_mul_pd(y,_mm_set1_pd(0.15))),_mm_set1_pd(2.0)));
		y = _mm_add_pd(y,_mm_mul_pd(Sin_batch2(_mm_mul_pd(z,_mm_set1_pd(0.15))),_mm_set1_pd(2.0)));
		z = _mm_add_pd(z,_mm_mul_pd(Sin_batch2(_mm_mul_pd(x,_mm_set1_pd(0.15))),_mm_set1_pd(2.0)));
		x = _mm_mul_pd(x,_mm_set1_pd(num));
		y = _mm_mul_pd(y,_mm_set1_pd(num2));
		z = _mm_mul_pd(z,_mm_set1_pd(num3));
		__m128d terrain = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(0.6)),_mm_mul_pd(y,_mm_set1_pd(0.6)),_mm_mul_pd(z,_mm_set1_pd(0.6)),4,0.5,1.8);
		terrain = PowFloat_batch2(_mm_mul_pd(_mm_add_pd(terrain,one),_mm_set1_pd(0.5)),1.3f);
		__m128d lowNoise = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(6.0)),_mm_mul_pd(y,_mm_set1_pd(6.0)),_mm_mul_pd(z,_mm_set1_pd(6.0)),5);
		lowNoise = Remap_batch2(-1.0,1.0,-0.1,0.15,lowNoise);
		__m128d tx = _mm_mul_pd(_mm_mul_pd(x,_mm_set1_pd(5.0)),_mm_set1_pd(3.0));
		__m128d ty = _mm_mul_pd(y,_mm_set1_pd(5.0));
		__m128d tz = _mm_mul_pd(z,_mm_set1_pd(5.0));
		__m128d warp = simplexNoise2.Noise3DFBM_batch2(tx,ty,tz,1);
		warp = _mm_mul_pd(warp,_mm_set1_pd(0.3));
		__m128d highNoise = simplexNoise2.Noise3DFBM_batch2(_mm_add_pd(tx,warp),_mm_add_pd(ty,warp),_mm_add_pd(tz,warp),5);
		highNoise = _mm_mul_pd(highNoise,_mm_set1_pd(0.1));
		terrain = _mm_cvtps_pd(_mm_cvtpd_ps(Levelize_batch2(Levelize_batch2(terrain,4))));
		terrain = _mm_min_pd(one,terrain);
		__m128d varied = _mm_blendv_pd(_mm_add_pd(terrain,lowNoise),_mm_add_pd(terrain,highNoise),_mm_cmp_pd(terrain,_mm_set1_pd(0.4),_CMP_GT_OQ));
		terrain = _mm_blendv_pd(varied,terrain,_mm_cmp_pd(terrain,_mm_set1_pd(0.8),_CMP_GT_OQ));
		__m128d height = Max_batch2(_mm_sub_pd(_mm_mul_pd(terrain,_mm_set1_pd(2.5)),_mm_mul_pd(terrain,peak)),_mm_mul_pd(lowNoise,_mm_set1_pd(2.0)));
		__m128d depth = _mm_div_pd(_mm_sub_pd(_mm_set1_pd(2.0),height),_mm_set1_pd(2.0));
		height = _mm_sub_pd(height,_mm_mul_pd(_mm_mul_pd(_mm_mul_pd(ridge,_mm_set1_pd(1.2)),envelope),depth));
		__m128d raised = _mm_add_pd(height,_mm_mul_pd(_mm_add_pd(_mm_mul_pd(noise,_mm_set1_pd(0.25)),_mm_mul_pd(detail,_mm_set1_pd(0.6))),depth));
		height = _mm_blendv_pd(height,raised,_mm_cmp_pd(height,zero,_CMP_GE_OQ));
		height = _mm_sub_pd(height,_mm_set1_pd(0.1));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.1)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d warpedX = Levelize_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.007)));
		__m256d warpedY = Levelize_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.007)));
		__m256d warpedZ = Levelize_batch4(_mm256_mul_pd(z,_mm256_set1_pd(0.007)));
		__m256d fx = _mm256_mul_pd(x,_mm256_set1_pd(0.05));
		__m256d fy = _mm256_mul_pd(y,_mm256_set1_pd(0.05));
		__m256d fz = _mm256_mul_pd(z,_mm256_set1_pd(0.05));
		warpedX = _mm256_add_pd(warpedX,_mm256_mul_pd(simplexNoise3.Noise_batch4(fx,fy,fz),_mm256_set1_pd(0.04)));
		warpedY = _mm256_add_pd(warpedY,_mm256_mul_pd(simplexNoise3.Noise_batch4(fy,fz,fx),_mm256_set1_pd(0.04)));
		warpedZ = _mm256_add_pd(warpedZ,_mm256_mul_pd(simplexNoise3.Noise_batch4(fz,fx,fy),_mm256_set1_pd(0.04)));
		const __m256d zero = _mm256_setzero_pd();
		const __m256d one = _mm256_set1_pd(1.0);
		const __m256d sign = _mm256_set1_pd(-0.0);
		__m256d noise = _mm256_andnot_pd(sign,simplexNoise4.Noise_batch4(warpedX,warpedY,warpedZ));
		__m256d ridge = _mm256_mul_pd(_mm256_sub_pd(_mm256_set1_pd(0.16),noise),_mm256_set1_pd(10.0));
		ridge = _mm256_and_pd(_mm256_min_pd(ridge,one),_mm256_cmp_pd(ridge,zero,_CMP_GT_OQ));
		ridge = _mm256_mul_pd(ridge,ridge);
		__m256d envelope = simplexNoise3.Noise3DFBM_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.005)),_mm256_mul_pd(z,_mm256_set1_pd(0.005)),_mm256_mul_pd(x,_mm256_set1_pd(0.005)),4);
		envelope = _mm256_mul_pd(_mm256_add_pd(envelope,_mm256_set1_pd(0.22)),_mm256_set1_pd(5.0));
		envelope = _mm256_and_pd(_mm256_min_pd(envelope,one),_mm256_cmp_pd(envelope,zero,_CMP_GT_OQ));
		__m256d detail = _mm256_andnot_pd(sign,simplexNoise4.Noise3DFBM_batch4(_mm256_mul_pd(warpedX,_mm256_set1_pd(1.5)),_mm256_mul_pd(warpedY,_mm256_set1_pd(1.5)),_mm256_mul_pd(warpedZ,_mm256_set1_pd(1.5)),2));

		__m256d craterNoise = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_set1_pd(5.0)),_mm256_mul_pd(_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_set1_pd(5.0)),_mm256_mul_pd(_mm256_mul_pd(z,_mm256_set1_pd(num3)),_mm256_set1_pd(5.0)),4);
		__m256d craterDetail = _mm256_mul_pd(craterNoise,_mm256_set1_pd(0.2));
		__m256d distanceScale = Remap_batch4(-1.0,1.0,0.2,5.0,craterNoise);
		__m256d peak = zero;
		for(int k=0;k<kCircleCount;k++) {
			__m256d dx = _mm256_sub_pd(_mm256_set1_pd((double)ellipses[k].x),x);
			__m256d dy = _mm256_sub_pd(_mm256_set1_pd((double)ellipses[k].y),y);
			__m256d dz = _mm256_sub_pd(_mm256_set1_pd((double)ellipses[k].z),z);
			__m256d distance = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(eccentricities[k]),dx),dx),_mm256_mul_pd(dy,dy)),_mm256_mul_pd(dz,dz));
			distance = _mm256_mul_pd(distanceScale,distance);
			// The radius square and square root are evaluated in float in the scalar path.
			__m256d width = _mm256_set1_pd((double)(ellipses[k].w * ellipses[k].w));
			__m256d inside = _mm256_cmp_pd(distance,width,_CMP_NGE_UQ);
			if(_mm256_movemask_pd(inside) == 0)
				continue;
			__m256d falloff = _mm256_cvtps_pd(_mm_sub_ps(_mm_set1_ps(1.0f),_mm_sqrt_ps(_mm256_cvtpd_ps(_mm256_div_pd(distance,width)))));
			__m256d t = _mm256_sub_pd(one,falloff);
			__m256d fourth = _mm256_mul_pd(_mm256_mul_pd(_mm256_mul_pd(t,t),t),t);
			__m256d shape = _mm256_add_pd(_mm256_sub_pd(one,fourth),_mm256_mul_pd(craterDetail,_mm256_set1_pd(2.0)));
			shape = _mm256_max_pd(zero,shape);
			peak = _mm256_blendv_pd(peak,Max_batch4(peak,_mm256_mul_pd(_mm256_set1_pd(heights[k]),shape)),inside);
		}
		x = _mm256_add_pd(x,_mm256_mul_pd(Sin_batch4(_mm256_mul_pd(y,_mm256_set1_pd(0.15))),_mm256_set1_pd(2.0)));
		y = _mm256_add_pd(y,_mm256_mul_pd(Sin_batch4(_mm256_mul_pd(z,_mm256_set1_pd(0.15))),_mm256_set1_pd(2.0)));
		z = _mm256_add_pd(z,_mm256_mul_pd(Sin_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.15))),_mm256_set1_pd(2.0)));
		x = _mm256_mul_pd(x,_mm256_set1_pd(num));
		y = _mm256_mul_pd(y,_mm256_set1_pd(num2));
		z = _mm256_mul_pd(z,_mm256_set1_pd(num3));
		__m256d terrain = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(0.6)),_mm256_mul_pd(y,_mm256_set1_pd(0.6)),_mm256_mul_pd(z,_mm256_set1_pd(0.6)),4,0.5,1.8);
		terrain = PowFloat_batch4(_mm256_mul_pd(_mm256_add_pd(terrain,one),_mm256_set1_pd(0.5)),1.3f);
		__m256d lowNoise = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(6.0)),_mm256_mul_pd(y,_mm256_set1_pd(6.0)),_mm256_mul_pd(z,_mm256_set1_pd(6.0)),5);
		lowNoise = Remap_batch4(-1.0,1.0,-0.1,0.15,lowNoise);
		__m256d tx = _mm256_mul_pd(_mm256_mul_pd(x,_mm256_set1_pd(5.0)),_mm256_set1_pd(3.0));
		__m256d ty = _mm256_mul_pd(y,_mm256_set1_pd(5.0));
		__m256d tz = _mm256_mul_pd(z,_mm256_set1_pd(5.0));
		__m256d warp = simplexNoise2.Noise3DFBM_batch4(tx,ty,tz,1);
		warp = _mm256_mul_pd(warp,_mm256_set1_pd(0.3));
		__m256d highNoise = simplexNoise2.Noise3DFBM_batch4(_mm256_add_pd(tx,warp),_mm256_add_pd(ty,warp),_mm256_add_pd(tz,warp),5);
		highNoise = _mm256_mul_pd(highNoise,_mm256_set1_pd(0.1));
		terrain = _mm256_cvtps_pd(_mm256_cvtpd_ps(Levelize_batch4(Levelize_batch4(terrain,4))));
		terrain = _mm256_min_pd(one,terrain);
		__m256d varied = _mm256_blendv_pd(_mm256_add_pd(terrain,lowNoise),_mm256_add_pd(terrain,highNoise),_mm256_cmp_pd(terrain,_mm256_set1_pd(0.4),_CMP_GT_OQ));
		terrain = _mm256_blendv_pd(varied,terrain,_mm256_cmp_pd(terrain,_mm256_set1_pd(0.8),_CMP_GT_OQ));
		__m256d height = Max_batch4(_mm256_sub_pd(_mm256_mul_pd(terrain,_mm256_set1_pd(2.5)),_mm256_mul_pd(terrain,peak)),_mm256_mul_pd(lowNoise,_mm256_set1_pd(2.0)));
		__m256d depth = _mm256_div_pd(_mm256_sub_pd(_mm256_set1_pd(2.0),height),_mm256_set1_pd(2.0));
		height = _mm256_sub_pd(height,_mm256_mul_pd(_mm256_mul_pd(_mm256_mul_pd(ridge,_mm256_set1_pd(1.2)),envelope),depth));
		__m256d raised = _mm256_add_pd(height,_mm256_mul_pd(_mm256_add_pd(_mm256_mul_pd(noise,_mm256_set1_pd(0.25)),_mm256_mul_pd(detail,_mm256_set1_pd(0.6))),depth));
		height = _mm256_blendv_pd(height,raised,_mm256_cmp_pd(height,zero,_CMP_GE_OQ));
		height = _mm256_sub_pd(height,_mm256_set1_pd(0.1));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.1)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		int num6 = dotNet35Random.Next();
		int num7 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num4);
		simplexNoise2 = SimplexNoise(num5);
		simplexNoise3 = SimplexNoise(num6);
		simplexNoise4 = SimplexNoise(num7);
		int num8 = dotNet35Random.Next();

		for(int i = 0; i < 10; i++) {
			VectorLF3 vectorLF = RandomTable::SphericNormal(num8,1.0);
			Vector4 vector = Vector4((float)vectorLF.x,(float)vectorLF.y,(float)vectorLF.z);
			vector.Normalize();
			vector *= NORMAL_PLANET_RADIUS;
			vector.w = (float)(dotNet35Random.NextDouble() * 10.0 + 40.0);
			ellipses[i] = vector;
			if(dotNet35Random.NextDouble() > 0.5)
			{
				eccentricities[i] = Remap(0.0,1.0,3.0,5.0,dotNet35Random.NextDouble());
			} else
			{
				eccentricities[i] = Remap(0.0,1.0,0.2,1.0 / 3.0,dotNet35Random.NextDouble());
			}
			heights[i] = Remap(0.0,1.0,1.0,2.0,dotNet35Random.NextDouble());
		}

		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain10");

			float float_buffer[40];
			double double_buffer[20];

			for(int i=0;i<10;i++) {
				const Vector4& t = ellipses[i];
				float_buffer[4*i] = t.x;
				float_buffer[4*i+1] = t.y;
				float_buffer[4*i+2] = t.z;
				float_buffer[4*i+3] = t.w;
			}
			for(int i=0;i<10;i++)
				double_buffer[i] = eccentricities[i];
			for(int i=0;i<10;i++)
				double_buffer[i+10] = heights[i];

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_3(),simplexNoise3.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_3(),simplexNoise3.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_4(),simplexNoise4.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_4(),simplexNoise4.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.float_buffer(),ellipses,sizeof(ellipses));
			double* ptr = tls.double_buffer();
			memcpy(ptr,eccentricities,sizeof(eccentricities));
			memcpy(ptr+10,heights,sizeof(heights));
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,tls.buffer);
			kernel.setArg(3,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}
};

class PlanetAlgorithm11: public PlanetAlgorithm
{
private:
	double Remap(double sourceMin,double sourceMax,double targetMin,double targetMax,double x)
	{
		return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
	}
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Levelize_batch2(__m128d value,int steps = 1,double level = 1.0) {
		value = _mm_div_pd(value,_mm_set1_pd(level));
		__m128d integral = _mm_floor_pd(value);
		__m128d fraction = _mm_sub_pd(value,integral);
		for(int step=0;step<steps;step++)
			fraction = _mm_mul_pd(_mm_mul_pd(_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm_mul_pd(_mm_add_pd(integral,fraction),_mm_set1_pd(level));
	}
	__forceinline __m128d __vectorcall Remap_batch2(double sourceMin,double sourceMax,double targetMin,double targetMax,__m128d x) {
		return _mm_add_pd(_mm_mul_pd(_mm_div_pd(_mm_sub_pd(x,_mm_set1_pd(sourceMin)),_mm_set1_pd(sourceMax-sourceMin)),_mm_set1_pd(targetMax-targetMin)),_mm_set1_pd(targetMin));
	}
	__forceinline __m128d __vectorcall Pow_batch2(__m128d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[2];
		_mm_storeu_pd(lanes,value);
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm_loadu_pd(lanes);
	}
	__forceinline __m256d __vectorcall Levelize_batch4(__m256d value,int steps = 1,double level = 1.0) {
		value = _mm256_div_pd(value,_mm256_set1_pd(level));
		__m256d integral = _mm256_floor_pd(value);
		__m256d fraction = _mm256_sub_pd(value,integral);
		for(int step=0;step<steps;step++)
			fraction = _mm256_mul_pd(_mm256_mul_pd(_mm256_sub_pd(_mm256_sub_pd(_mm256_set1_pd(3.0),fraction),fraction),fraction),fraction);
		return _mm256_mul_pd(_mm256_add_pd(integral,fraction),_mm256_set1_pd(level));
	}
	__forceinline __m256d __vectorcall Remap_batch4(double sourceMin,double sourceMax,double targetMin,double targetMax,__m256d x) {
		return _mm256_add_pd(_mm256_mul_pd(_mm256_div_pd(_mm256_sub_pd(x,_mm256_set1_pd(sourceMin)),_mm256_set1_pd(sourceMax-sourceMin)),_mm256_set1_pd(targetMax-targetMin)),_mm256_set1_pd(targetMin));
	}
	__forceinline __m256d __vectorcall Pow_batch4(__m256d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[4];
		_mm256_storeu_pd(lanes,value);
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm256_loadu_pd(lanes);
	}
#endif
public:
	static constexpr double num = 0.007;
	static constexpr double num2 = 0.007;
	static constexpr double num3 = 0.007;

	double modY,num4,num5,num6;
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;
	SimplexNoise simplexNoise3;

	void GenerateHeight1(int index) override {
		double num10 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num11 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num12 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double num15 = simplexNoise2.Noise3DFBM(num10 * num * 4.0,num11 * num2 * 8.0,num12 * num3 * 4.0,3);
		double x = simplexNoise.Noise3DFBM(num10 * num * 0.6,num11 * num * 1.5 * 2.5,num12 * num * 0.6,6,0.45,1.8) * 0.95 + num15 * 0.05;
		x = Remap(-1.0,1.0,0.0,1.0,x);
		x = Math.Pow(x,modY);
		x += 1.0;
		x = Maths::Levelize2(x);
		double x2 = simplexNoise3.Noise3DFBM(num10 * num4,num11 * num5,num12 * num6,5,0.55);
		x2 = Remap(-1.0,1.0,0.0,1.0,x2);
		x2 = Math.Pow(x2,0.65);
		double num14 = Maths::Levelize3(x2) * x;
		double num13 = (num14 - 0.4) * 0.9;
		num13 = Math.Max(-0.3,num13);
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num13) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d secondary = simplexNoise2.Noise3DFBM_batch2(_mm_mul_pd(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_set1_pd(4.0)),_mm_mul_pd(_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_set1_pd(8.0)),_mm_mul_pd(_mm_mul_pd(z,_mm_set1_pd(num3)),_mm_set1_pd(4.0)),3);
		__m128d base = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_set1_pd(0.6)),_mm_mul_pd(_mm_mul_pd(_mm_mul_pd(y,_mm_set1_pd(num)),_mm_set1_pd(1.5)),_mm_set1_pd(2.5)),_mm_mul_pd(_mm_mul_pd(z,_mm_set1_pd(num)),_mm_set1_pd(0.6)),6,0.45,1.8);
		base = _mm_add_pd(_mm_mul_pd(base,_mm_set1_pd(0.95)),_mm_mul_pd(secondary,_mm_set1_pd(0.05)));
		base = Remap_batch2(-1.0,1.0,0.0,1.0,base);
		base = Pow_batch2(base,modY);
		base = Levelize_batch2(_mm_add_pd(base,_mm_set1_pd(1.0)),2);
		__m128d detail = simplexNoise3.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(num4)),_mm_mul_pd(y,_mm_set1_pd(num5)),_mm_mul_pd(z,_mm_set1_pd(num6)),5,0.55);
		detail = Remap_batch2(-1.0,1.0,0.0,1.0,detail);
		detail = Pow_batch2(detail,0.65);
		__m128d height = _mm_mul_pd(Levelize_batch2(detail,3),base);
		height = _mm_mul_pd(_mm_sub_pd(height,_mm_set1_pd(0.4)),_mm_set1_pd(0.9));
		height = _mm_max_pd(_mm_set1_pd(-0.3),height);
		height = _mm_mul_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d secondary = simplexNoise2.Noise3DFBM_batch4(_mm256_mul_pd(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_set1_pd(4.0)),_mm256_mul_pd(_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_set1_pd(8.0)),_mm256_mul_pd(_mm256_mul_pd(z,_mm256_set1_pd(num3)),_mm256_set1_pd(4.0)),3);
		__m256d base = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_set1_pd(0.6)),_mm256_mul_pd(_mm256_mul_pd(_mm256_mul_pd(y,_mm256_set1_pd(num)),_mm256_set1_pd(1.5)),_mm256_set1_pd(2.5)),_mm256_mul_pd(_mm256_mul_pd(z,_mm256_set1_pd(num)),_mm256_set1_pd(0.6)),6,0.45,1.8);
		base = _mm256_add_pd(_mm256_mul_pd(base,_mm256_set1_pd(0.95)),_mm256_mul_pd(secondary,_mm256_set1_pd(0.05)));
		base = Remap_batch4(-1.0,1.0,0.0,1.0,base);
		base = Pow_batch4(base,modY);
		base = Levelize_batch4(_mm256_add_pd(base,_mm256_set1_pd(1.0)),2);
		__m256d detail = simplexNoise3.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(num4)),_mm256_mul_pd(y,_mm256_set1_pd(num5)),_mm256_mul_pd(z,_mm256_set1_pd(num6)),5,0.55);
		detail = Remap_batch4(-1.0,1.0,0.0,1.0,detail);
		detail = Pow_batch4(detail,0.65);
		__m256d height = _mm256_mul_pd(Levelize_batch4(detail,3),base);
		height = _mm256_mul_pd(_mm256_sub_pd(height,_mm256_set1_pd(0.4)),_mm256_set1_pd(0.9));
		height = _mm256_max_pd(_mm256_set1_pd(-0.3),height);
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		double modX = planet.mod_x;
		modY = planet.mod_y;
		num4 = 0.002 * modX;
		num5 = 0.002 * modX * 4.0;
		num6 = 0.002 * modX;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num7 = dotNet35Random.Next();
		int num8 = dotNet35Random.Next();
		int num9 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num7);
		simplexNoise2 = SimplexNoise(num8);
		simplexNoise3 = SimplexNoise(num9);
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain11");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_3(),simplexNoise3.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_3(),simplexNoise3.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,num4);
			kernel.setArg(3,num5);
			kernel.setArg(4,num6);
			kernel.setArg(5,modY);
			kernel.setArg(6,tls.buffer);
			kernel.setArg(7,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}

	bool check_vein_position(const Vector3& target_pos,EVeinType vein_type) override {
		float target_height = QueryHeight(target_pos);
		return target_height < NORMAL_PLANET_RADIUS
			|| (vein_type == EVeinType::Oil && target_height < NORMAL_PLANET_RADIUS + 0.5f)
			|| ((int)vein_type <= 2 && target_height > NORMAL_PLANET_RADIUS + 0.7f)
			|| ((vein_type == EVeinType::Silicium || vein_type == EVeinType::Titanium) && target_height <= NORMAL_PLANET_RADIUS + 0.7f);
	}
};

class PlanetAlgorithm12: public PlanetAlgorithm
{
private:
	double Remap(double sourceMin,double sourceMax,double targetMin,double targetMax,double x)
	{
		return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
	}
	double CurveEvaluate(double t)
	{
		t /= 0.6;
		if(t >= 1.0)
		{
			return 0.0;
		}
		return Math.Pow(1.0 - t,3.0) + Math.Pow(1.0 - t,2.0) * 3.0 * t;
	}
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Remap_batch2(double sourceMin,double sourceMax,double targetMin,double targetMax,__m128d x) {
		return _mm_add_pd(_mm_mul_pd(_mm_div_pd(_mm_sub_pd(x,_mm_set1_pd(sourceMin)),_mm_set1_pd(sourceMax-sourceMin)),_mm_set1_pd(targetMax-targetMin)),_mm_set1_pd(targetMin));
	}
	__forceinline __m128d __vectorcall Pow_batch2(__m128d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[2];
		_mm_storeu_pd(lanes,value);
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm_loadu_pd(lanes);
	}
	__forceinline __m128d __vectorcall Asin_batch2(__m128d value) {
		double lanes[2];
		_mm_storeu_pd(lanes,value);
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Math.Asin(lanes[lane]);
		return _mm_loadu_pd(lanes);
	}
	__forceinline __m128d __vectorcall CurveEvaluate_batch2(__m128d t) {
		t = _mm_div_pd(t,_mm_set1_pd(0.6));
		__m128d remaining = _mm_sub_pd(_mm_set1_pd(1.0),t);
		__m128d curve = _mm_add_pd(Pow_batch2(remaining,3.0),_mm_mul_pd(_mm_mul_pd(Pow_batch2(remaining,2.0),_mm_set1_pd(3.0)),t));
		return _mm_blendv_pd(curve,_mm_setzero_pd(),_mm_cmp_pd(t,_mm_set1_pd(1.0),_CMP_GE_OQ));
	}
	__forceinline __m256d __vectorcall Remap_batch4(double sourceMin,double sourceMax,double targetMin,double targetMax,__m256d x) {
		return _mm256_add_pd(_mm256_mul_pd(_mm256_div_pd(_mm256_sub_pd(x,_mm256_set1_pd(sourceMin)),_mm256_set1_pd(sourceMax-sourceMin)),_mm256_set1_pd(targetMax-targetMin)),_mm256_set1_pd(targetMin));
	}
	__forceinline __m256d __vectorcall Pow_batch4(__m256d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[4];
		_mm256_storeu_pd(lanes,value);
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm256_loadu_pd(lanes);
	}
	__forceinline __m256d __vectorcall Asin_batch4(__m256d value) {
		double lanes[4];
		_mm256_storeu_pd(lanes,value);
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Math.Asin(lanes[lane]);
		return _mm256_loadu_pd(lanes);
	}
	__forceinline __m256d __vectorcall CurveEvaluate_batch4(__m256d t) {
		t = _mm256_div_pd(t,_mm256_set1_pd(0.6));
		__m256d remaining = _mm256_sub_pd(_mm256_set1_pd(1.0),t);
		__m256d curve = _mm256_add_pd(Pow_batch4(remaining,3.0),_mm256_mul_pd(_mm256_mul_pd(Pow_batch4(remaining,2.0),_mm256_set1_pd(3.0)),t));
		return _mm256_blendv_pd(curve,_mm256_setzero_pd(),_mm256_cmp_pd(t,_mm256_set1_pd(1.0),_CMP_GE_OQ));
	}
#endif
public:
	static constexpr double num2 = 0.2;
	static constexpr double num3 = 8.0;

	double num,modY;
	SimplexNoise simplexNoise;
	SimplexNoise simplexNoise2;

	void GenerateHeight1(int index) override {
		double num6 = Math.Abs(Math.Asin(vertices[index].y)) * 2.0 / Math.PI;
		double num11 = vertices[index].x;
		double num12 = (double)vertices[index].y * 2.5 * modY;
		double num13 = vertices[index].z;
		double num14 = simplexNoise2.Noise3DFBM(num11 * num,num12 * num,num13 * num,3,0.4) * 0.2;
		double num9 = simplexNoise.RidgedNoise(num11 * num,num12 * num - num14,num13 * num,6,0.7,2.0,0.8);
		double num10 = simplexNoise.Noise3DFBM(num11 * num,num12 * num - num14,num13 * num,6,0.6,2.0,0.7);
		num10 *= num9 + num10;
		num10 = num2 + num3 * num10 * num9;
		double x = num10 + 0.5;
		x = Remap(-8.0,8.0,0.0,1.0,x);
		x = Maths::Clamp01(x);
		x += 0.5;
		x = Math.Pow(x,1.5);
		x -= CurveEvaluate((float)(num6 * 0.9));
		double num7 = Maths::Clamp(x * 2.0,0.0,2.0);
		num7 = num7 * 1.1 - 0.2;
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num7) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Widen raw coordinates.
		__m128d x = _mm_cvtps_pd(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f));
		__m128d y = _mm_cvtps_pd(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f));
		__m128d z = _mm_cvtps_pd(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f));
		__m128d latitude = _mm_div_pd(_mm_mul_pd(_mm_andnot_pd(_mm_set1_pd(-0.0),Asin_batch2(y)),_mm_set1_pd(2.0)),_mm_set1_pd(Math.PI));
		y = _mm_mul_pd(_mm_mul_pd(y,_mm_set1_pd(2.5)),_mm_set1_pd(modY));
		__m128d fx = _mm_mul_pd(x,_mm_set1_pd(num));
		__m128d fy = _mm_mul_pd(y,_mm_set1_pd(num));
		__m128d fz = _mm_mul_pd(z,_mm_set1_pd(num));
		__m128d warp = _mm_mul_pd(simplexNoise2.Noise3DFBM_batch2(fx,fy,fz,3,0.4),_mm_set1_pd(0.2));
		fy = _mm_sub_pd(fy,warp);
		__m128d ridge = simplexNoise.RidgedNoise_batch2(fx,fy,fz,6,0.7,2.0,0.8);
		__m128d terrain = simplexNoise.Noise3DFBM_batch2(fx,fy,fz,6,0.6,2.0,0.7);
		terrain = _mm_mul_pd(terrain,_mm_add_pd(ridge,terrain));
		terrain = _mm_add_pd(_mm_set1_pd(num2),_mm_mul_pd(_mm_mul_pd(_mm_set1_pd(num3),terrain),ridge));
		terrain = Remap_batch2(-8.0,8.0,0.0,1.0,_mm_add_pd(terrain,_mm_set1_pd(0.5)));
		terrain = _mm_min_pd(_mm_set1_pd(1.0),_mm_max_pd(_mm_setzero_pd(),terrain));
		terrain = Pow_batch2(_mm_add_pd(terrain,_mm_set1_pd(0.5)),1.5);
		// CurveEvaluate receives a double widened from float in GenerateHeight1.
		latitude = _mm_cvtps_pd(_mm_cvtpd_ps(_mm_mul_pd(latitude,_mm_set1_pd(0.9))));
		terrain = _mm_sub_pd(terrain,CurveEvaluate_batch2(latitude));
		__m128d height = _mm_min_pd(_mm_set1_pd(2.0),_mm_max_pd(_mm_setzero_pd(),_mm_mul_pd(terrain,_mm_set1_pd(2.0))));
		height = _mm_sub_pd(_mm_mul_pd(height,_mm_set1_pd(1.1)),_mm_set1_pd(0.2));
		height = _mm_mul_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Widen raw coordinates.
		__m256d x = _mm256_cvtps_pd(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x));
		__m256d y = _mm256_cvtps_pd(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y));
		__m256d z = _mm256_cvtps_pd(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z));
		__m256d latitude = _mm256_div_pd(_mm256_mul_pd(_mm256_andnot_pd(_mm256_set1_pd(-0.0),Asin_batch4(y)),_mm256_set1_pd(2.0)),_mm256_set1_pd(Math.PI));
		y = _mm256_mul_pd(_mm256_mul_pd(y,_mm256_set1_pd(2.5)),_mm256_set1_pd(modY));
		__m256d fx = _mm256_mul_pd(x,_mm256_set1_pd(num));
		__m256d fy = _mm256_mul_pd(y,_mm256_set1_pd(num));
		__m256d fz = _mm256_mul_pd(z,_mm256_set1_pd(num));
		__m256d warp = _mm256_mul_pd(simplexNoise2.Noise3DFBM_batch4(fx,fy,fz,3,0.4),_mm256_set1_pd(0.2));
		fy = _mm256_sub_pd(fy,warp);
		__m256d ridge = simplexNoise.RidgedNoise_batch4(fx,fy,fz,6,0.7,2.0,0.8);
		__m256d terrain = simplexNoise.Noise3DFBM_batch4(fx,fy,fz,6,0.6,2.0,0.7);
		terrain = _mm256_mul_pd(terrain,_mm256_add_pd(ridge,terrain));
		terrain = _mm256_add_pd(_mm256_set1_pd(num2),_mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(num3),terrain),ridge));
		terrain = Remap_batch4(-8.0,8.0,0.0,1.0,_mm256_add_pd(terrain,_mm256_set1_pd(0.5)));
		terrain = _mm256_min_pd(_mm256_set1_pd(1.0),_mm256_max_pd(_mm256_setzero_pd(),terrain));
		terrain = Pow_batch4(_mm256_add_pd(terrain,_mm256_set1_pd(0.5)),1.5);
		// CurveEvaluate receives a double widened from float in GenerateHeight1.
		latitude = _mm256_cvtps_pd(_mm256_cvtpd_ps(_mm256_mul_pd(latitude,_mm256_set1_pd(0.9))));
		terrain = _mm256_sub_pd(terrain,CurveEvaluate_batch4(latitude));
		__m256d height = _mm256_min_pd(_mm256_set1_pd(2.0),_mm256_max_pd(_mm256_setzero_pd(),_mm256_mul_pd(terrain,_mm256_set1_pd(2.0))));
		height = _mm256_sub_pd(_mm256_mul_pd(height,_mm256_set1_pd(1.1)),_mm256_set1_pd(0.2));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		double modX = planet.mod_x;
		modY = planet.mod_y;
		num = 1.1 * modX;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		simplexNoise = SimplexNoise(num4);
		simplexNoise2 = SimplexNoise(num5);
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain12");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			memcpy(tls.perm_buffer_2(),simplexNoise2.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_2(),simplexNoise2.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,num);
			kernel.setArg(3,modY);
			kernel.setArg(4,tls.buffer);
			kernel.setArg(5,tls.heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}

	bool check_vein_position(const Vector3& target_pos,EVeinType vein_type) override {
		float target_height = QueryHeight(target_pos);
		return target_height < NORMAL_PLANET_RADIUS || (vein_type == EVeinType::Oil && target_height < NORMAL_PLANET_RADIUS + 0.5f) || (vein_type == EVeinType::Fireice && target_height < NORMAL_PLANET_RADIUS + 1.2f);
	}
};

class PlanetAlgorithm13: public PlanetAlgorithm
{
private:
	double Remap(double sourceMin,double sourceMax,double targetMin,double targetMax,double x)
	{
		return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
	}
#ifdef SUPPORT_AVX2
	__forceinline __m128d __vectorcall Remap_batch2(double sourceMin,double sourceMax,double targetMin,double targetMax,__m128d x) {
		return _mm_add_pd(_mm_mul_pd(_mm_div_pd(_mm_sub_pd(x,_mm_set1_pd(sourceMin)),_mm_set1_pd(sourceMax-sourceMin)),_mm_set1_pd(targetMax-targetMin)),_mm_set1_pd(targetMin));
	}
	__forceinline __m128d __vectorcall Pow_batch2(__m128d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[2];
		_mm_storeu_pd(lanes,value);
		for(int lane=0;lane<2;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm_loadu_pd(lanes);
	}
	__forceinline __m256d __vectorcall Remap_batch4(double sourceMin,double sourceMax,double targetMin,double targetMax,__m256d x) {
		return _mm256_add_pd(_mm256_mul_pd(_mm256_div_pd(_mm256_sub_pd(x,_mm256_set1_pd(sourceMin)),_mm256_set1_pd(sourceMax-sourceMin)),_mm256_set1_pd(targetMax-targetMin)),_mm256_set1_pd(targetMin));
	}
	__forceinline __m256d __vectorcall Pow_batch4(__m256d value,double exponent) {
		// AVX2 has no pow instruction; preserve the existing libm result per lane.
		double lanes[4];
		_mm256_storeu_pd(lanes,value);
		for(int lane=0;lane<4;lane++)
			lanes[lane] = Math.Pow(lanes[lane],exponent);
		return _mm256_loadu_pd(lanes);
	}
#endif
public:
	double num,num2,num3,modY;
	SimplexNoise simplexNoise;

	void GenerateHeight1(int index) override {
		double num4 = vertices[index].x * NORMAL_PLANET_RADIUS;
		double num5 = vertices[index].y * NORMAL_PLANET_RADIUS;
		double num6 = vertices[index].z * NORMAL_PLANET_RADIUS;
		double x = Remap(-1.0,1.0,0.0,1.0,simplexNoise.Noise3DFBM(num4 * num,num5 * num2,num6 * num3,6));
		x = Math.Pow(x,modY) * 3.0625;
		x = Remap(0.0,2.0,0.0,4.0,x);
		if(x < 1.0)
			x = Math.Pow(x,2.0);
		x -= 0.2;
		double num8 = Math.Min(x,4.0);
		if(num8 > 2.0)
			num8 = ((!(num8 > 3.0)) ? (2.0 - 1.0 * (num8 - 2.0)) : ((!(num8 > 3.5)) ? 1.0 : (1.0 + 2.0 * (num8 - 3.5))));
		heightData[index] = (unsigned short)(((double)NORMAL_PLANET_RADIUS + num8 + 0.1) * 100.0);
	}

#ifdef SUPPORT_AVX2
	void GenerateHeight2(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m128d x = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d y = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d z = _mm_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,0.0f,0.0f),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m128d terrain = simplexNoise.Noise3DFBM_batch2(_mm_mul_pd(x,_mm_set1_pd(num)),_mm_mul_pd(y,_mm_set1_pd(num2)),_mm_mul_pd(z,_mm_set1_pd(num3)),6);
		terrain = Remap_batch2(-1.0,1.0,0.0,1.0,terrain);
		terrain = _mm_mul_pd(Pow_batch2(terrain,modY),_mm_set1_pd(3.0625));
		terrain = Remap_batch2(0.0,2.0,0.0,4.0,terrain);
		terrain = _mm_blendv_pd(terrain,Pow_batch2(terrain,2.0),_mm_cmp_pd(terrain,_mm_set1_pd(1.0),_CMP_LT_OQ));
		terrain = _mm_sub_pd(terrain,_mm_set1_pd(0.2));
		__m128d height = _mm_min_pd(terrain,_mm_set1_pd(4.0));
		__m128d descending = _mm_sub_pd(_mm_set1_pd(2.0),_mm_mul_pd(_mm_set1_pd(1.0),_mm_sub_pd(height,_mm_set1_pd(2.0))));
		__m128d rising = _mm_add_pd(_mm_set1_pd(1.0),_mm_mul_pd(_mm_set1_pd(2.0),_mm_sub_pd(height,_mm_set1_pd(3.5))));
		__m128d upper = _mm_blendv_pd(_mm_set1_pd(1.0),rising,_mm_cmp_pd(height,_mm_set1_pd(3.5),_CMP_GT_OQ));
		upper = _mm_blendv_pd(descending,upper,_mm_cmp_pd(height,_mm_set1_pd(3.0),_CMP_GT_OQ));
		height = _mm_blendv_pd(height,upper,_mm_cmp_pd(height,_mm_set1_pd(2.0),_CMP_GT_OQ));
		height = _mm_mul_pd(_mm_add_pd(_mm_add_pd(_mm_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm_set1_pd(0.1)),_mm_set1_pd(100.0));
		double values[2];
		_mm_storeu_pd(values,height);
		for(int lane=0;lane<2;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}

	void GenerateHeight4(int* index_arr) override {
		// Multiply coordinates in float before widening, matching the scalar path.
		__m256d x = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].x,vertices[index_arr[1]].x,vertices[index_arr[2]].x,vertices[index_arr[3]].x),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d y = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].y,vertices[index_arr[1]].y,vertices[index_arr[2]].y,vertices[index_arr[3]].y),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d z = _mm256_cvtps_pd(_mm_mul_ps(_mm_setr_ps(vertices[index_arr[0]].z,vertices[index_arr[1]].z,vertices[index_arr[2]].z,vertices[index_arr[3]].z),_mm_set1_ps(NORMAL_PLANET_RADIUS)));
		__m256d terrain = simplexNoise.Noise3DFBM_batch4(_mm256_mul_pd(x,_mm256_set1_pd(num)),_mm256_mul_pd(y,_mm256_set1_pd(num2)),_mm256_mul_pd(z,_mm256_set1_pd(num3)),6);
		terrain = Remap_batch4(-1.0,1.0,0.0,1.0,terrain);
		terrain = _mm256_mul_pd(Pow_batch4(terrain,modY),_mm256_set1_pd(3.0625));
		terrain = Remap_batch4(0.0,2.0,0.0,4.0,terrain);
		terrain = _mm256_blendv_pd(terrain,Pow_batch4(terrain,2.0),_mm256_cmp_pd(terrain,_mm256_set1_pd(1.0),_CMP_LT_OQ));
		terrain = _mm256_sub_pd(terrain,_mm256_set1_pd(0.2));
		__m256d height = _mm256_min_pd(terrain,_mm256_set1_pd(4.0));
		__m256d descending = _mm256_sub_pd(_mm256_set1_pd(2.0),_mm256_mul_pd(_mm256_set1_pd(1.0),_mm256_sub_pd(height,_mm256_set1_pd(2.0))));
		__m256d rising = _mm256_add_pd(_mm256_set1_pd(1.0),_mm256_mul_pd(_mm256_set1_pd(2.0),_mm256_sub_pd(height,_mm256_set1_pd(3.5))));
		__m256d upper = _mm256_blendv_pd(_mm256_set1_pd(1.0),rising,_mm256_cmp_pd(height,_mm256_set1_pd(3.5),_CMP_GT_OQ));
		upper = _mm256_blendv_pd(descending,upper,_mm256_cmp_pd(height,_mm256_set1_pd(3.0),_CMP_GT_OQ));
		height = _mm256_blendv_pd(height,upper,_mm256_cmp_pd(height,_mm256_set1_pd(2.0),_CMP_GT_OQ));
		height = _mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_set1_pd((double)NORMAL_PLANET_RADIUS),height),_mm256_set1_pd(0.1)),_mm256_set1_pd(100.0));
		double values[4];
		_mm256_storeu_pd(values,height);
		for(int lane=0;lane<4;lane++)
			heightData[index_arr[lane]] = (unsigned short)values[lane];
	}
#endif

	void GenerateTerrain(const PlanetClassSimple& planet,bool gen_terr = false) override {
		double modX = planet.mod_x;
		modY = planet.mod_y;
		num = 0.007 * modX;
		num2 = 0.007 * modX;
		num3 = 0.007 * modX;
		simplexNoise = SimplexNoise(DotNet35Random(planet.seed).Next());
		heightData.assign(VERTICES_DATALENGTH,0);
		if(gen_terr && OpenCLManager::get_worker()) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain13");

			ThreadLocalBuffers& tls = get_tls_buffers();
			memcpy(tls.perm_buffer_1(),simplexNoise.perm,tls.PERM_BUF_SIZE);
			memcpy(tls.permMod12_buffer_1(),simplexNoise.permMod12,tls.PERM_BUF_SIZE);
			tls.upload_buffer();

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,NORMAL_PLANET_RADIUS);
			kernel.setArg(2,num);
			kernel.setArg(3,num2);
			kernel.setArg(4,num3);
			kernel.setArg(5,modY);
			kernel.setArg(6,tls.buffer);
			kernel.setArg(7,tls.heightData_buffer);
			
			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
			cl_int err = tls.queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			tls.download_buffer(heightData.data());
			OpenCLManager::return_worker();
			for(int i=LAND_DATALENGTH-1;i>=0;i--) {
				heightData[landIndex[i]] = heightData[i];
				heightData[i] = 0;
			}
		}
	}

	bool check_vein_position(const Vector3& target_pos,EVeinType vein_type) override {
		float target_height = QueryHeight(target_pos);
		return target_height < NORMAL_PLANET_RADIUS || (vein_type == EVeinType::Oil && target_height < NORMAL_PLANET_RADIUS + 0.5f) || ((int)vein_type <= 4 && target_height > NORMAL_PLANET_RADIUS + 0.7f);
	}
};

//这个类目前完全用不到，先放着不用管
//class PlanetAlgorithm14: public PlanetAlgorithm
//{
//public:
//	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
//		double num = 0.007;
//		double num2 = 0.007;
//		double num3 = 0.007;
//		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
//		int num4 = dotNet35Random.Next();
//		int num5 = dotNet35Random.Next();
//		int num6 = dotNet35Random.Next();
//		int num7 = dotNet35Random.Next();
//		SimplexNoise simplexNoise = SimplexNoise(num4);
//		SimplexNoise simplexNoise2 = SimplexNoise(num5);
//		SimplexNoise simplexNoise3 = SimplexNoise(num6);
//		SimplexNoise simplexNoise4 = SimplexNoise(num7);
//		PlanetRawData& data = planet.data;
//		data.heightData.resize(DATALENGTH);
//		//data.debugData.resize(DATALENGTH);
//		if(OpenCLManager::SUPPORT_GPU && OpenCLManager::SUPPORT_DOUBLE) {
//			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain14");
//
//			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise.perm);
//			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise2.perm);
//			cl::Buffer perm_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise3.perm);
//			cl::Buffer perm_buffer_4(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise4.perm);
//			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise.permMod12);
//			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise2.permMod12);
//			cl::Buffer permMod12_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise3.permMod12);
//			cl::Buffer permMod12_buffer_4(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int) * PERM_LENGTH,simplexNoise4.permMod12);
//			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);
//
//			kernel.setArg(0,OpenCLManager::vertices_buffer);
//			kernel.setArg(1,sizeof(float),&planet.radius);
//			kernel.setArg(2,perm_buffer_1);
//			kernel.setArg(3,perm_buffer_2);
//			kernel.setArg(4,perm_buffer_3);
//			kernel.setArg(5,perm_buffer_4);
//			kernel.setArg(6,permMod12_buffer_1);
//			kernel.setArg(7,permMod12_buffer_2);
//			kernel.setArg(8,permMod12_buffer_3);
//			kernel.setArg(9,permMod12_buffer_4);
//			kernel.setArg(10,heightData_buffer);
//			//kernel.setArg(11,OpenCLManager::debugData_buffer);
//
//			int local_size = OpenCLManager::local_size;
//			int global_size = (int)ceil((double)LAND_DATALENGTH/local_size) * local_size;
//			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
//			OpenCLManager::queue.finish();
//			if(err != CL_SUCCESS){
//				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
//				throw std::runtime_error("Kernel execution failed");
//			}
//
//			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
//						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
//			//OpenCLManager::queue.enqueueReadBuffer(OpenCLManager::debugData_buffer,CL_TRUE,0,
//			//			  sizeof(float) * data.debugData.size(),data.debugData.data());
//		} else {
//			for(int i = 0; i < DATALENGTH; i++)
//			{
//				double num8 = data.vertices[i].x * planet.radius;
//				double num9 = data.vertices[i].y * planet.radius;
//				double num10 = data.vertices[i].z * planet.radius;
//				double num11 = Maths::Levelize(num8 * 0.007 / 2.0);
//				double num12 = Maths::Levelize(num9 * 0.007 / 2.0);
//				double num13 = Maths::Levelize(num10 * 0.007 / 2.0);
//				num11 += simplexNoise3.Noise(num8 * 0.05,num9 * 0.05,num10 * 0.05) * 0.04;
//				num12 += simplexNoise3.Noise(num9 * 0.05,num10 * 0.05,num8 * 0.05) * 0.04;
//				num13 += simplexNoise3.Noise(num10 * 0.05,num8 * 0.05,num9 * 0.05) * 0.04;
//				double num14 = Math.Abs(simplexNoise4.Noise(num11,num12,num13));
//				double num15 = (0.12 - num14) * 10.0;
//				num15 = ((!(num15 > 0.0)) ? 0.0 : ((num15 > 1.0) ? 1.0 : num15));
//				num15 *= num15;
//				double num16 = (simplexNoise3.Noise3DFBM(num9 * 0.005,num10 * 0.005,num8 * 0.005,4) + 0.22) * 5.0;
//				num16 = ((!(num16 > 0.0)) ? 0.0 : ((num16 > 1.0) ? 1.0 : num16));
//				Math.Abs(simplexNoise4.Noise3DFBM(num11 * 1.5,num12 * 1.5,num13 * 1.5,2));
//				num8 += Math.Sin(num9 * 0.15) * 3.0;
//				num9 += Math.Sin(num10 * 0.15) * 3.0;
//				num10 += Math.Sin(num8 * 0.15) * 3.0;
//				double num17 = 0.0;
//				double num18 = 0.0;
//				double num19 = simplexNoise.Noise3DFBM(num8 * num * 1.0,num9 * num2 * 1.1,num10 * num3 * 1.0,6,0.5,1.8);
//				double num20 = simplexNoise2.Noise3DFBM(num8 * num * 1.3 + 0.5,num9 * num2 * 2.8 + 0.2,num10 * num3 * 1.3 + 0.7,3) * 2.0;
//				double num21 = simplexNoise2.Noise3DFBM(num8 * num * 6.0,num9 * num2 * 12.0,num10 * num3 * 6.0,2) * 2.0;
//				double num22 = simplexNoise2.Noise3DFBM(num8 * num * 0.8,num9 * num2 * 0.8,num10 * num3 * 0.8,2) * 2.0;
//				double num23 = num19 * 2.0 + 0.92;
//				double num24 = num20 * (double)Mathf.Abs((float)num22 + 0.5f);
//				num23 += (double)Mathf.Clamp01((float)(num24 - 0.35) * 1.0f);
//				if(num23 < 0.0)
//				{
//					num23 = 0.0;
//				}
//				double num25 = num23;
//				num25 = Maths::Levelize2(num23);
//				if(num25 > 0.0)
//				{
//					num25 = Maths::Levelize2(num23);
//					num25 = Maths::Levelize4(num25);
//				}
//				double num26 = ((!(num25 > 0.0)) ? ((double)Mathf.Lerp(-4.0f,0.0f,(float)num25 + 1.0f)) : ((!(num25 > 1.0)) ? ((double)Mathf.Lerp(0.0f,0.3f,(float)num25) + num21 * 0.1) : ((num25 > 2.0) ? ((double)Mathf.Lerp(1.4f,2.7f,(float)num25 - 2.0f) + num21 * 0.12) : ((double)Mathf.Lerp(0.3f,1.4f,(float)num25 - 1.0f) + num21 * 0.12))));
//				if(num23 < 0.0)
//				{
//					num23 *= 2.0;
//				}
//				if(num23 < 1.0)
//				{
//					num23 = Maths::Levelize(num23);
//				}
//				num17 -= num15 * 1.2 * num16;
//				//data.debugData[i] = (float)(num25 <= 0.0) + (float)(num25 <= 1.0) + (float)(num25 > 2.0);
//				if(num17 >= 0.0)
//				{
//					num17 = num26;
//				}
//				num17 -= 0.1;
//				num18 = Mathf.Abs((float)num23);
//				double x = Mathf.Clamp01((float)((0.0 - num17 + 2.0) / 2.5));
//				x = Math.Pow(x,10.0);
//				num18 = (1.0 - x) * num18 + x * 2.0;
//				num18 = ((!(num18 > 0.0)) ? 0.0 : ((num18 > 2.0) ? 2.0 : num18));
//				num18 += ((num18 > 1.8) ? ((0.0 - num21) * 0.8) : (num21 * 0.2)) * (1.0 - x);
//				double num27 = -0.3 - num17;
//				if(num27 > 0.0)
//				{
//					double num28 = simplexNoise2.Noise(num8 * 0.16,num9 * 0.16,num10 * 0.16) - 1.0;
//					num27 = ((num27 > 1.0) ? 1.0 : num27);
//					num27 = (3.0 - num27 - num27) * num27 * num27;
//					num17 = -0.3 - num27 * 10.0 + num27 * num27 * num27 * num27 * num28 * 0.5;
//				}
//				data.heightData[i] = (unsigned short)(((double)planet.radius + num17 + 0.2) * 100.0);
//			}
//		}
//	}
//};

namespace planet_algorithm_detail {
template<class Algorithm,class Fn>
inline void run_algorithm(Fn& fn) {
	Algorithm algorithm{};
	fn(algorithm);
}

template<class Fn>
inline void with_algorithm(int algoId,Fn&& fn) {
	switch(algoId) {
	case 1:
		run_algorithm<PlanetAlgorithm1>(fn);
		return;
	case 2:
		run_algorithm<PlanetAlgorithm2>(fn);
		return;
	case 3:
		run_algorithm<PlanetAlgorithm3>(fn);
		return;
	case 4:
		run_algorithm<PlanetAlgorithm4>(fn);
		return;
	case 5:
		run_algorithm<PlanetAlgorithm5>(fn);
		return;
	case 6:
		run_algorithm<PlanetAlgorithm6>(fn);
		return;
	case 7:
		run_algorithm<PlanetAlgorithm7>(fn);
		return;
	case 8:
		run_algorithm<PlanetAlgorithm8>(fn);
		return;
	case 9:
		run_algorithm<PlanetAlgorithm9>(fn);
		return;
	case 10:
		run_algorithm<PlanetAlgorithm10>(fn);
		return;
	case 11:
		run_algorithm<PlanetAlgorithm11>(fn);
		return;
	case 12:
		run_algorithm<PlanetAlgorithm12>(fn);
		return;
	case 13:
		run_algorithm<PlanetAlgorithm13>(fn);
		return;
	default:
		run_algorithm<PlanetAlgorithm0>(fn);
		return;
	}
}
}

inline void generate_planet_veins(const GalaxyClass& galaxy,const StarClass& star,PlanetClass& planet) {
	planet_algorithm_detail::with_algorithm(planet.algoId,[&](auto& algorithm) {
		algorithm.get_veins(galaxy,star,planet);
	});
}
inline void generate_planet_veins(PlanetClassSimple& planet) {
	planet_algorithm_detail::with_algorithm(planet.algoId,[&](auto& algorithm) {
		algorithm.GenerateTerrain(planet);
		algorithm.GenerateVeins(planet,planet.star->galaxy->birthPlanetId);
	});
}

//inline unique_ptr<PlanetAlgorithm> GetPlanetAlgorithm(int algoId) {
//	switch(algoId) {
//	case 1:
//	return make_unique<PlanetAlgorithm1>();
//	case 2:
//	return make_unique<PlanetAlgorithm2>();
//	case 3:
//	return make_unique<PlanetAlgorithm3>();
//	case 4:
//	return make_unique<PlanetAlgorithm4>();
//	case 5:
//	return make_unique<PlanetAlgorithm5>();
//	case 6:
//	return make_unique<PlanetAlgorithm6>();
//	case 7:
//	return make_unique<PlanetAlgorithm7>();
//	case 8:
//	return make_unique<PlanetAlgorithm8>();
//	case 9:
//	return make_unique<PlanetAlgorithm9>();
//	case 10:
//	return make_unique<PlanetAlgorithm10>();
//	case 11:
//	return make_unique<PlanetAlgorithm11>();
//	case 12:
//	return make_unique<PlanetAlgorithm12>();
//	case 13:
//	return make_unique<PlanetAlgorithm13>();
//	//case 14:
//	//	return make_unique<PlanetAlgorithm14>();
//	default:
//	return make_unique<PlanetAlgorithm0>();
//	}
//}
