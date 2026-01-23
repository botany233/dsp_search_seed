#pragma once
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <glm/glm.hpp>
#include <CL/opencl.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "defines.hpp"
#include "LDB.hpp"
#include "util.hpp"
#include "Maths.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "DotNet35Random.hpp"
#include "const_value.hpp"
#include "SimplexNoise.hpp"
#include "RandomTable.hpp"
#include "PlanetRawData.hpp"

#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4838)

class OpenCLManager
{
public:
	static bool SUPPORT_GPU;
	static bool SUPPORT_DOUBLE;
	static int local_size;
	static int device_id;
	static std::vector<cl::Device> devices;
	static std::vector<std::string> devices_info;
	static cl::Context context;
	static cl::CommandQueue queue;
	static cl::Program program;
	static cl::Buffer vertices_buffer;
	//static cl::Buffer custom_buffer;
	//static cl::Buffer perm_buffer_1;
	//static cl::Buffer perm_buffer_2;
	//static cl::Buffer perm_buffer_3;
	//static cl::Buffer perm_buffer_4;
	//static cl::Buffer permMod12_buffer_1;
	//static cl::Buffer permMod12_buffer_2;
	//static cl::Buffer permMod12_buffer_3;
	//static cl::Buffer permMod12_buffer_4;
	//static cl::Buffer heightData_buffer;
	//static cl::Buffer debugData_buffer;

	static void do_init()
	{
		static bool is_init = false;
		if(is_init)
			return;
		is_init = true;
		init_device(-1);
		set_local_size();
	}

	static bool init_device(int input_device_id = 0) {
		SUPPORT_GPU = false;
		devices.clear();
		devices_info.clear();
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		for(const cl::Platform& plat : platforms) {
			std::vector<cl::Device> devs;
			plat.getDevices(CL_DEVICE_TYPE_GPU,&devs);
			std::string plat_name = plat.getInfo<CL_PLATFORM_NAME>();
			for(const cl::Device& dev : devs) {
				devices.push_back(dev);
				std::string dev_name = dev.getInfo<CL_DEVICE_NAME>();
				devices_info.push_back(plat_name + " " + dev_name);
			}
		}
		if(input_device_id < 0 || devices.size() <= input_device_id) {
			return false;
		}
		device_id = input_device_id;
		cl::Device& device = devices[device_id];

		// 检查是否支持double类型
		SUPPORT_DOUBLE = false;
		try {
			std::string extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();

			// 检查cl_khr_fp64扩展
			if(extensions.find("cl_khr_fp64") != std::string::npos) {
				SUPPORT_DOUBLE = true;
			}

			// 检查cl_amd_fp64扩展（AMD特有）
			if(extensions.find("cl_amd_fp64") != std::string::npos) {
				SUPPORT_DOUBLE = true;
			}

			// 直接尝试获取double精度信息
			try {
				cl_device_fp_config doubleConfig = device.getInfo<CL_DEVICE_DOUBLE_FP_CONFIG>();
				if(doubleConfig != 0) {
					SUPPORT_DOUBLE = true;
				}
			} catch(...) {
				// 如果获取double配置失败，继续检查其他方式
			}
		} catch(...) {
			// 扩展信息获取失败时的处理
		}

		// 创建上下文和命令队列
		context = cl::Context(device);
		queue = cl::CommandQueue(context,device);

		// 创建程序
		cl::Program::Sources sources;
		if(SUPPORT_DOUBLE)
			AddSources(sources,"assets/generate_terrain_double.cl");
		else
			AddSources(sources,"assets/generate_terrain.cl");
		program = cl::Program(context,sources);
		cl_int buildResult = program.build({device});

		// 检查构建状态
		if(buildResult != CL_SUCCESS) {
			std::cerr << "Program build failed with error code: " << buildResult << std::endl;

			// 获取构建日志
			std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
			std::cerr << "Build Log:\n" << buildLog << std::endl;

			throw std::runtime_error("Program build failed");
		}

		vector<float> vertices(DATALENGTH*3);
		for(int i=0;i<DATALENGTH;i++) {
			vertices[i*3] = PlanetRawData::vertices[i].x;
			vertices[i*3+1] = PlanetRawData::vertices[i].y;
			vertices[i*3+2] = PlanetRawData::vertices[i].z;
		}
		
		vertices_buffer = cl::Buffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float) * vertices.size(),vertices.data());
		//custom_buffer = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(float) * 512);
		//perm_buffer_1 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//perm_buffer_2 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//perm_buffer_3 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//perm_buffer_4 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//permMod12_buffer_1 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//permMod12_buffer_2 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//permMod12_buffer_3 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//permMod12_buffer_4 = cl::Buffer(context,CL_MEM_READ_ONLY,sizeof(short) * PERM_LENGTH);
		//heightData_buffer = cl::Buffer(context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);
		//debugData_buffer = cl::Buffer(context,CL_MEM_WRITE_ONLY,sizeof(float) * DATALENGTH);

		SUPPORT_GPU = true;
		return true;
	}

	static void set_local_size(int size = 256) {
		if(size < 32)
			size = 32;
		local_size = size;
	}

	static void AddSources(cl::Program::Sources& sources,const string& file_name) {
		ifstream file(file_name);
		string* source_code = new string(istreambuf_iterator<char>(file),(istreambuf_iterator<char>()));
		sources.push_back((*source_code).c_str());
	}
};

class PlanetAlgorithm
{
protected:
	std::vector<Vector3> veinVectors = std::vector<Vector3>(512, Vector3());
	std::vector<EVeinType> veinVectorTypes = std::vector<EVeinType>(512, EVeinType::None_vein);
	std::vector<Vector2> tmp_vecs;
	int veinVectorCount = 0;

public:
	glm::vec3 vector3_to_glm(const Vector3& vec)
	{
		return glm::vec3(vec.x,vec.y,vec.z);
	}

	Vector3 glm_to_vector3(const glm::vec3& vec)
	{
		return Vector3(vec.x,vec.y,vec.z);
	}

	virtual void GenerateTerrain(PlanetClass& planet, double modX, double modY) = 0;

	virtual void GenerateVeins(StarClass& star, PlanetClass& planet, int birthPlanetId, int* veins,int* res) {
		ThemeProto themeProto = LDB.Select(planet.theme);
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		int birthSeed = dotNet35Random.Next();
		DotNet35Random dotNet35Random2 = DotNet35Random(dotNet35Random.Next());
		PlanetRawData& rawData = planet.data;
		float num = 2.1f / planet.radius;
		int array[15] = {0};
		float array2[15] = {0};
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
		float p = 1.0f;
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
			break;
		}
		}
		for(int n = 0; n < themeProto.RareVeins.size(); n++)
		{
			int num2 = themeProto.RareVeins[n];
			float num3 = ((star.index == 0) ? themeProto.RareSettings[n * 4] : themeProto.RareSettings[n * 4 + 1]);
			float num4 = themeProto.RareSettings[n * 4 + 2];
			float num5 = themeProto.RareSettings[n * 4 + 3];
			float num6 = num5;
			num3 = 1.0f - Mathf.Pow(1.0f - num3,p);
			num5 = 1.0f - Mathf.Pow(1.0f - num5,p);
			num6 = 1.0f - Mathf.Pow(1.0f - num6,p);
			if(!(dotNet35Random.NextDouble() < (double)num3))
			{
				continue;
			}
			array[num2]++;
			array2[num2] = num5;
			for(int num7 = 1; num7 < 12; num7++)
			{
				if(dotNet35Random.NextDouble() >= (double)num4)
				{
					break;
				}
				array[num2]++;
			}
		}
		bool flag = birthPlanetId == planet.id;
		if(flag)
		{
			planet.GenBirthPoints(rawData,birthSeed);
		}
		veinVectorCount = 0;
		Vector3 birthPoint;
		if(flag)
		{
			birthPoint = planet.birthPoint;
			birthPoint.Normalize();
			birthPoint *= 0.75f;
		}
		else
		{
			birthPoint.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.y = (float)dotNet35Random2.NextDouble() - 0.5f;
			birthPoint.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.Normalize();
			birthPoint *= (float)(dotNet35Random2.NextDouble() * 0.4 + 0.2);
		}
		if(flag)
		{
			veinVectorTypes[0] = EVeinType::Iron;
			veinVectors[0] = planet.birthResourcePoint0;
			veinVectorTypes[1] = EVeinType::Copper;
			veinVectors[1] = planet.birthResourcePoint1;
			veinVectorCount = 2;
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
					float target_height = rawData.QueryHeight(target_pos);
					if(target_height < planet.radius || (eVeinType == EVeinType::Oil && target_height < planet.radius + 0.5f))
					{
						continue;
					}
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
		tmp_vecs.clear();
		for(int vein_group_index = 0; vein_group_index < veinVectorCount; vein_group_index++)
		{
			tmp_vecs.clear();
			Vector3 normalized = Vector3::Normalize(veinVectors[vein_group_index]);
			EVeinType eVeinType2 = veinVectorTypes[vein_group_index];
			int vein_point_type = (int)eVeinType2;
			veins[vein_point_type-1]++;
			glm::quat quaternion = glm::rotation(vector3_to_glm(Vector3::up()),vector3_to_glm(normalized));
			Vector3 vector = glm_to_vector3(quaternion * vector3_to_glm(Vector3::right()));
			Vector3 vector2 = glm_to_vector3(quaternion * vector3_to_glm(Vector3::forward()));
			//if(planet.id == 201)
			//{
			//	cout << std::setprecision(7);
			//	cout << "矿簇: " << vein_group_index << endl;
			//	cout << vector.x << " " << vector.y << " " << vector.z << endl;
			//	cout << vector2.x << " " << vector2.y << " " << vector2.z << endl;
			//}
			tmp_vecs.push_back(Vector2::zero());
			int vein_point_num = Mathf.RoundToInt(array2[vein_point_type] * (float)dotNet35Random2.Next(20,25));
			if(eVeinType2 == EVeinType::Oil)
			{
				vein_point_num = 1;
			}
			if(flag && vein_group_index < 2)
			{
				vein_point_num = 6;
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
			//if(planet.id == 201)
			//{
			//	std::cout<< std::setprecision(7);
			//	std::cout << "矿物: " << vein_point_type << " " << vein_point_num << std::endl;
			//	std::cout << veinVectors[vein_group_index].x << " " << veinVectors[vein_group_index].y << " " << veinVectors[vein_group_index].z << std::endl;
			//	for(int i = 0; i< tmp_vecs.size(); i++) {
			//		std::cout << tmp_vecs[i].x << " " << tmp_vecs[i].y << " ";
			//	}
			//	std::cout << std::endl;
			//}
			for(int vein_point_index = 0; vein_point_index < tmp_vecs.size(); vein_point_index++)
			{
				Vector3 vector5 = (vector * tmp_vecs[vein_point_index].x + vector2 * tmp_vecs[vein_point_index].y) * num;
				dotNet35Random2.Next();
				Vector3 vein_pos = normalized + vector5;
				//TODO: 这里对油井坐标未变换！
				//if(vein.type == EVeinType::Oil)
				//{
				//	vein.pos = planet.aux.RawSnap(vein.pos);
				//}
				float num29 = rawData.QueryHeight(vein_pos);
				//if(planet.id == 201)
				//{
				//	cout << "real_pos: " << vein_pos.x << " " << vein_pos.y << " " << vein_pos.z << ", height: " << num29 << endl;
				//}
				if(planet.waterItemId == 0 || num29 >= planet.radius)
				{
					res[vein_point_type-1]++;
				}
			}
		}
		//std::cout << "矿脉生成完成" << std::endl;
		tmp_vecs.clear();
		//if(planet.id == 201)
		//{
		//	cout << planet.radius << endl;
		//}
	};
};

class PlanetAlgorithm0: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet, double modX, double modY) override
	{
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain0");

			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,sizeof(float),&planet.radius);
			kernel.setArg(1,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		}
		else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				data.heightData[i] = (unsigned short)((double)planet.radius * 100.0);
			}
		}
	}

	void GenerateVeins(StarClass& star,PlanetClass& planet,int birthPlanetId,int* veins,int* res) override {
		//do nothing
	}
};

class PlanetAlgorithm1: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override
	{
		double num = 0.01;
		double num2 = 0.012;
		double num3 = 0.01;
		double num4 = 3.0;
		double num5 = -0.2;
		double num6 = 0.9;
		double num7 = 0.5;
		double num8 = 2.5;
		double num9 = 0.3;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num10 = dotNet35Random.Next();
		int num11 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num10);
		SimplexNoise simplexNoise2 = SimplexNoise(num11);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain1");

			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,sizeof(float),&planet.radius);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		}
		else {
			for(int i = 0; i < DATALENGTH; i++) {
				double num12 = data.vertices[i].x * planet.radius;
				double num13 = data.vertices[i].y * planet.radius;
				double num14 = data.vertices[i].z * planet.radius;
				double num15 = 0.0;
				double num16 = 0.0;
				double num17 = simplexNoise.Noise3DFBM(num12 * num,num13 * num2,num14 * num3,6) * num4 + num5;
				double num18 = simplexNoise2.Noise3DFBM(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3) * num4 * num6 + num7;
				double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
				double num20 = num17 + num19;
				double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
				double num22 = ((num21 > 0.0) ? Maths::Levelize3(num21,0.7) : Maths::Levelize2(num21,0.5));
				double num23 = simplexNoise2.Noise3DFBM(num12 * num * 2.5,num13 * num2 * 8.0,num14 * num3 * 2.5,2) * 0.6 - 0.3;
				double num24 = num21 * num8 + num23 + num9;
				double num25 = ((num24 < 1.0) ? num24 : ((num24 - 1.0) * 0.8 + 1.0));
				num15 = num22;
				num16 = num25;
				data.heightData[i] = (unsigned short)(((double)planet.radius + num15 + 0.2) * 100.0);
			}
		}
	}
};

class PlanetAlgorithm2: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override
	{
		modX = (3.0 - modX - modX) * modX * modX;
		double num = 0.0035;
		double num2 = 0.025 * modX + 0.0035 * (1.0 - modX);
		double num3 = 0.0035;
		double num4 = 3.0;
		double num5 = 1.0 + 1.3 * modY;
		num *= num5;
		num2 *= num5;
		num3 *= num5;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num6 = dotNet35Random.Next();
		int num7 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num6);
		SimplexNoise simplexNoise2 = SimplexNoise(num7);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain2");

			float custom[4] = {planet.radius,num,num2,num3};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num8 = data.vertices[i].x * planet.radius;
				double num9 = data.vertices[i].y * planet.radius;
				double num10 = data.vertices[i].z * planet.radius;
				double num11 = data.vertices[i].y;
				double num12 = 0.0;
				double num13 = 0.0;
				double num14 = simplexNoise.Noise3DFBM(num8 * num,num9 * num2,num10 * num3,6,0.45,1.8);
				double num15 = simplexNoise2.Noise3DFBM(num8 * num * 2.0,num9 * num2 * 2.0,num10 * num3 * 2.0,3);
				double value = num14 * num4 + num4 * 0.4;
				double num16 = 0.6 / (Math.Abs(value) + 0.6) - 0.25;
				double num17 = ((num16 < 0.0) ? (num16 * 0.3) : num16);
				double num18 = Math.Pow(Math.Abs(num11 * 1.01),3.0) * 1.0;
				double num19 = ((num15 < 0.0) ? 0.0 : num15);
				double num20 = ((num18 > 1.0) ? 1.0 : num18);
				num12 = num17;
				num13 = num17 * 1.5 + num19 * 1.0 + num20;
				data.heightData[i] = (unsigned short)(((double)planet.radius + num12 + 0.1) * 100.0);
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
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.007;
		double num2 = 0.007;
		double num3 = 0.007;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num4);
		SimplexNoise simplexNoise2 = SimplexNoise(num5);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		//data.debugData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU && OpenCLManager::SUPPORT_DOUBLE) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain3");

			float custom[2] = {planet.radius,modX};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);
			//kernel.setArg(7,OpenCLManager::debugData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			//OpenCLManager::queue.enqueueReadBuffer(OpenCLManager::debugData_buffer,CL_TRUE,0,
			//			  sizeof(float) * data.debugData.size(),data.debugData.data());

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num6 = data.vertices[i].x * planet.radius;
				double num7 = data.vertices[i].y * planet.radius;
				double num8 = data.vertices[i].z * planet.radius;
				num6 += Math.Sin(num7 * 0.15) * 3.0;
				num7 += Math.Sin(num8 * 0.15) * 3.0;
				num8 += Math.Sin(num6 * 0.15) * 3.0;
				double num9 = 0.0;
				double num10 = 0.0;
				double num11 = simplexNoise.Noise3DFBM(num6 * num * 1.0,num7 * num2 * 1.1,num8 * num3 * 1.0,6,0.5,1.8);
				double num12 = simplexNoise2.Noise3DFBM(num6 * num * 1.3 + 0.5,num7 * num2 * 2.8 + 0.2,num8 * num3 * 1.3 + 0.7,3) * 2.0;
				double num13 = simplexNoise2.Noise3DFBM(num6 * num * 6.0,num7 * num2 * 12.0,num8 * num3 * 6.0,2) * 2.0;
				num13 = Lerp(num13,num13 * 0.1,modX);
				double num14 = simplexNoise2.Noise3DFBM(num6 * num * 0.8,num7 * num2 * 0.8,num8 * num3 * 0.8,2) * 2.0;
				double num15 = num11 * 2.0 + 0.92;
				double num16 = num12 * (double)Mathf.Abs((float)num14 + 0.5f);
				num15 += (double)Mathf.Clamp01((float)(num16 - 0.35) * 1.0f);
				if(num15 < 0.0)
				{
					num15 *= 2.0;
				}
				double num17 = num15;
				num17 = Maths::Levelize2(num15);
				if(num17 > 0.0)
				{
					num17 = Maths::Levelize2(num15);
					num17 = Lerp(Maths::Levelize4(num17),num17,modX);
				}
				double b = ((!(num17 > 0.0)) ? ((double)Mathf.Lerp(-1.0f,0.0f,(float)num17 + 1.0f)) : ((!(num17 > 1.0)) ? ((double)Mathf.Lerp(0.0f,0.3f,(float)num17) + num13 * 0.1) : ((num17 > 2.0) ? ((double)Mathf.Lerp(1.2f,2.0f,(float)num17 - 2.0f) + num13 * 0.12) : ((double)Mathf.Lerp(0.3f,1.2f,(float)num17 - 1.0f) + num13 * 0.12))));
				double a = ((!(num17 > 0.0)) ? ((double)Mathf.Lerp(-4.0f,0.0f,(float)num17 + 1.0f)) : ((!(num17 > 1.0)) ? ((double)Mathf.Lerp(0.0f,0.3f,(float)num17) + num13 * 0.1) : ((num17 > 2.0) ? ((double)Mathf.Lerp(1.4f,2.7f,(float)num17 - 2.0f) + num13 * 0.12) : ((double)Mathf.Lerp(0.3f,1.4f,(float)num17 - 1.0f) + num13 * 0.12))));
				double num18 = Lerp(a,b,modX);
				if(num15 < 0.0)
				{
					num15 *= 2.0;
				}
				if(num15 < 1.0)
				{
					num15 = Maths::Levelize(num15);
				}
				num9 = num18;
				num10 = Mathf.Abs((float)num15);
				num10 = ((num10 > 0.0) ? ((num10 > 2.0) ? 2.0 : num10) : 0.0);
				num10 += ((num10 > 1.8) ? ((0.0 - num13) * 0.8) : (num13 * 0.2));
				data.heightData[i] = (unsigned short)(((double)planet.radius + num9 + 0.2) * 100.0);
				//data.debugData[i] = num18;
			}
		}
	}
};

class PlanetAlgorithm4: public PlanetAlgorithm
{
private:
	static constexpr int kCircleCount = 80;
	Vector4 circles[80] = {};
	double heights[80] = {};

public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.007;
		double num2 = 0.007;
		double num3 = 0.007;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num4);
		SimplexNoise simplexNoise2 = SimplexNoise(num5);
		int num6 = dotNet35Random.Next();
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain4");

			float custom[401];
			custom[0] = planet.radius;

			for(int i = 0; i < 80; i++)
			{
				VectorLF3 vectorLF = RandomTable::SphericNormal(num6,1.0);
				Vector4 vector = Vector4((float)vectorLF.x,(float)vectorLF.y,(float)vectorLF.z);
				vector.Normalize();
				vector *= planet.radius;
				vector.w = (float)vectorLF.magnitude() * 8.0f + 8.0f;
				vector.w *= vector.w;
				custom[4*i+81] = vector.x;
				custom[4*i+82] = vector.y;
				custom[4*i+83] = vector.z;
				custom[4*i+84] = vector.w;
				custom[i+1] = dotNet35Random.NextDouble() * 0.4 + 0.20000000298023224;
			}

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < 80; i++)
			{
				VectorLF3 vectorLF = RandomTable::SphericNormal(num6,1.0);
				Vector4 vector = Vector4((float)vectorLF.x,(float)vectorLF.y,(float)vectorLF.z);
				vector.Normalize();
				vector *= planet.radius;
				vector.w = (float)vectorLF.magnitude() * 8.0f + 8.0f;
				vector.w *= vector.w;
				circles[i] = vector;
				heights[i] = dotNet35Random.NextDouble() * 0.4 + 0.20000000298023224;
			}
			for(int j = 0; j < DATALENGTH; j++)
			{
				double num7 = data.vertices[j].x * planet.radius;
				double num8 = data.vertices[j].y * planet.radius;
				double num9 = data.vertices[j].z * planet.radius;
				double num10 = 0.0;
				double num11 = 0.0;
				double num12 = simplexNoise.Noise3DFBM(num7 * num,num8 * num2,num9 * num3,4,0.45,1.8);
				double num13 = simplexNoise2.Noise3DFBM(num7 * num * 5.0,num8 * num2 * 5.0,num9 * num3 * 5.0,4);
				double num14 = num12 * 1.5;
				double num15 = num13 * 0.2;
				double num16 = num14 * 0.08 + num15 * 2.0;
				double num17 = 0.0;
				for(int k = 0; k < 80; k++)
				{
					double num18 = (double)circles[k].x - num7;
					double num19 = (double)circles[k].y - num8;
					double num20 = (double)circles[k].z - num9;
					double num21 = num18 * num18 + num19 * num19 + num20 * num20;
					if(!(num21 > (double)circles[k].w))
					{
						double num22 = num21 / (double)circles[k].w + num15 * 1.2;
						if(num22 < 0.0)
						{
							num22 = 0.0;
						}
						double num23 = num22 * num22;
						double num24 = num23 * num22;
						double num25 = -15.0 * num24 + 21.833333333334 * num23 - 7.533333333333 * num22 + 0.7 + num15;
						if(num25 < 0.0)
						{
							num25 = 0.0;
						}
						num25 *= num25;
						num25 *= heights[k];
						num17 = ((num17 > num25) ? num17 : num25);
					}
				}
				num10 = num17 + num16 + 0.2;
				num11 = num14 * 2.0 + 0.8;
				num11 = ((num11 > 2.0) ? 2.0 : ((num11 < 0.0) ? 0.0 : num11));
				num11 += ((num11 > 1.5) ? (0.0 - num17) : num17) * 0.5;
				num11 += num13 * 0.63;
				data.heightData[j] = (unsigned short)(((double)planet.radius + num10 + 0.1) * 100.0);
			}
		}
	}
};

class PlanetAlgorithm5: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num = dotNet35Random.Next();
		int num2 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num);
		SimplexNoise simplexNoise2 = SimplexNoise(num2);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain5");

			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,sizeof(float),&planet.radius);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num3 = data.vertices[i].x * planet.radius;
				double num4 = data.vertices[i].y * planet.radius;
				double num5 = data.vertices[i].z * planet.radius;
				double num6 = 0.0;
				double num7 = 0.0;
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
				double num15 = simplexNoise.Noise3DFBM(num5 * 0.06,num4 * 0.06,num3 * 0.06,2);
				num6 -= num12 * 1.2 * num13;
				if(num6 >= 0.0)
				{
					num6 += num11 * 0.25 + num14 * 0.6;
				}
				num6 -= 0.1;
				num7 = num11 * 2.1;
				if(num7 < 0.0)
				{
					num7 *= 5.0;
				}
				num7 = ((!(num7 > -1.0)) ? (-1.0) : ((num7 > 2.0) ? 2.0 : num7));
				num7 += num15 * 0.6 * num7;
				double num16 = -0.3 - num6;
				if(num16 > 0.0)
				{
					double num17 = simplexNoise2.Noise(num3 * 0.16,num4 * 0.16,num5 * 0.16) - 1.0;
					num16 = ((num16 > 1.0) ? 1.0 : num16);
					num16 = (3.0 - num16 - num16) * num16 * num16;
					num6 = -0.3 - num16 * 3.700000047683716 + num16 * num16 * num16 * num16 * num17 * 0.5;
				}
				data.heightData[i] = (unsigned short)(((double)planet.radius + num6 + 0.2) * 100.0);
			}
		}
	}
};

class PlanetAlgorithm6: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num = dotNet35Random.Next();
		int num2 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num);
		SimplexNoise simplexNoise2 = SimplexNoise(num2);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain6");

			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,sizeof(float),&planet.radius);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num3 = data.vertices[i].x * planet.radius;
				double num4 = data.vertices[i].y * planet.radius;
				double num5 = data.vertices[i].z * planet.radius;
				double num6 = 0.0;
				double num7 = 0.0;
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
				num6 -= num12 * 1.2 * num13;
				if(num6 >= 0.0)
				{
					num6 += num11 * 0.25 + num14 * 0.6;
				}
				num6 -= 0.1;
				double num15 = -0.3 - num6;
				if(num15 > 0.0)
				{
					num15 = ((num15 > 1.0) ? 1.0 : num15);
					num15 = (3.0 - num15 - num15) * num15 * num15;
					num6 = -0.3 - num15 * 3.700000047683716;
				}
				double f = ((num12 > 0.30000001192092896) ? num12 : 0.30000001192092896);
				f = Maths::Levelize(f,0.7);
				num6 = ((num6 > -0.800000011920929) ? num6 : ((0.0 - f - num11) * 0.8999999761581421));
				num6 = ((num6 > -1.2000000476837158) ? num6 : (-1.2000000476837158));
				num7 = num6 * num12;
				num7 += num11 * 2.1 + 0.800000011920929;
				if(num7 > 1.7000000476837158 && num7 < 2.0)
				{
					num7 = 2.0;
				}
				data.heightData[i] = (unsigned short)(((double)planet.radius + num6 + 0.2) * 100.0);
			}
		}
	}
};

class PlanetAlgorithm7: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.008;
		double num2 = 0.01;
		double num3 = 0.01;
		double num4 = 3.0;
		double num5 = -2.4;
		double num6 = 0.9;
		double num7 = 0.5;
		double num8 = 2.5;
		double num9 = 0.3;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num10 = dotNet35Random.Next();
		int num11 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num10);
		SimplexNoise simplexNoise2 = SimplexNoise(num11);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain7");

			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,sizeof(float),&planet.radius);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num12 = data.vertices[i].x * planet.radius;
				double num13 = data.vertices[i].y * planet.radius;
				double num14 = data.vertices[i].z * planet.radius;
				double num15 = 0.0;
				double num16 = 0.0;
				double num17 = simplexNoise.Noise3DFBM(num12 * num,num13 * num2,num14 * num3,6) * num4 + num5;
				double num18 = simplexNoise2.Noise3DFBM(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3) * num4 * num6 + num7;
				double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
				double num20 = num17 + num19;
				double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
				double num22 = ((num21 > 0.0) ? Maths::Levelize3(num21,0.7) : Maths::Levelize2(num21,0.5));
				double num23 = simplexNoise2.Noise3DFBM(num12 * num * 2.5,num13 * num2 * 8.0,num14 * num3 * 2.5,2) * 0.6 - 0.3;
				double num24 = num21 * num8 + num23 + num9;
				double num25 = ((num24 < 1.0) ? num24 : ((num24 - 1.0) * 0.8 + 1.0));
				num15 = num22;
				num16 = num25;
				data.heightData[i] = (unsigned short)(((double)planet.radius + num15) * 100.0);
			}
		}
	}

	void GenerateVeins(StarClass& star,PlanetClass& planet,int birthPlanetId,int* veins,int* res) override {
		ThemeProto themeProto = LDB.Select(planet.theme);
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		DotNet35Random dotNet35Random2 = DotNet35Random(dotNet35Random.Next());
		PlanetRawData& data = planet.data;
		float num = 2.1f / planet.radius;
		int array[15] = {0};
		float array2[15] = {0};
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
		float p = 1.0f;
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
			break;
		}
		}
		for(int n = 0; n < themeProto.RareVeins.size(); n++)
		{
			int num2 = themeProto.RareVeins[n];
			float num3 = ((star.index == 0) ? themeProto.RareSettings[n * 4] : themeProto.RareSettings[n * 4 + 1]);
			float num4 = themeProto.RareSettings[n * 4 + 2];
			float num5 = themeProto.RareSettings[n * 4 + 3];
			float num6 = num5;
			num3 = 1.0f - Mathf.Pow(1.0f - num3,p);
			num5 = 1.0f - Mathf.Pow(1.0f - num5,p);
			num6 = 1.0f - Mathf.Pow(1.0f - num6,p);
			if(!(dotNet35Random.NextDouble() < (double)num3))
			{
				continue;
			}
			array[num2]++;
			array2[num2] = num5;
			for(int num7 = 1; num7 < 12; num7++)
			{
				if(dotNet35Random.NextDouble() >= (double)num4)
				{
					break;
				}
				array[num2]++;
			}
		}
		bool flag = birthPlanetId == planet.id;
		veinVectorCount = 0;
		Vector3 birthPoint;
		birthPoint.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
		birthPoint.y = (float)dotNet35Random2.NextDouble() - 0.5f;
		birthPoint.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
		birthPoint.Normalize();
		birthPoint *= (float)(dotNet35Random2.NextDouble() * 0.4 + 0.2);
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
					if(eVeinType == EVeinType::Bamboo && data.QueryHeight(target_pos) > planet.realRadius() - 4.0f)
					{
						continue;
					}
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
		tmp_vecs.clear();
		for(int vein_group_index = 0; vein_group_index < veinVectorCount; vein_group_index++)
		{
			tmp_vecs.clear();
			Vector3 normalized = Vector3::Normalize(veinVectors[vein_group_index]);
			EVeinType eVeinType2 = veinVectorTypes[vein_group_index];
			int vein_point_type = (int)eVeinType2;
			veins[vein_point_type-1]++;
			glm::quat quaternion = glm::rotation(vector3_to_glm(Vector3::up()),vector3_to_glm(normalized));
			Vector3 vector = glm_to_vector3(quaternion * vector3_to_glm(Vector3::right()));
			Vector3 vector2 = glm_to_vector3(quaternion * vector3_to_glm(Vector3::forward()));
			tmp_vecs.push_back(Vector2::zero());
			int vein_point_num = Mathf.RoundToInt(array2[vein_point_type] * (float)dotNet35Random2.Next(20,25));
			if(eVeinType2 == EVeinType::Oil)
			{
				vein_point_num = 1;
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
			for(int vein_point_index = 0; vein_point_index < tmp_vecs.size(); vein_point_index++)
			{
				//Vector3 vector5 = (vector * tmp_vecs[vein_point_index].x + vector2 * tmp_vecs[vein_point_index].y) * num;
				dotNet35Random2.Next();
				//Vector3 vein_pos = normalized + vector5;
				//TODO: 这里对油井未对齐！
				//if(vein.type == EVeinType::Oil)
				//{
				//	vein.pos = planet.aux.RawSnap(vein.pos);
				//}
				//float num29 = data.QueryHeight(vein_pos);
				res[vein_point_type-1]++;
			}
		}
		tmp_vecs.clear();
	};
};

class PlanetAlgorithm8: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.002 * modX;
		double num2 = 0.002 * modX * modX * 6.66667;
		double num3 = 0.002 * modX;
		SimplexNoise simplexNoise = SimplexNoise(DotNet35Random(planet.seed).Next());
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain8");

			float custom[5] = {planet.radius,num,num2,num3,modY};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,permMod12_buffer_1);
			kernel.setArg(4,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num4 = data.vertices[i].x * planet.radius;
				double num5 = data.vertices[i].y * planet.radius;
				double num6 = data.vertices[i].z * planet.radius;
				double num7 = 0.0;
				double num8 = 0.0;
				float num9 = Mathf.Clamp((float)simplexNoise.Noise3DFBM(num4 * num,num5 * num2,num6 * num3,6,0.45,1.8) + 1.0f + (float)modY * 0.01f,0.0f,2.0f);
				float num10 = 0.0f;
				if((double)num9 < 1.0)
				{
					float f = Mathf.Cos(num9 * MATHF_PI) * 1.1f;
					f = Mathf.Sign(f) * Mathf.Pow(f,4.0f);
					f = Mathf.Clamp(f,-1.0f,1.0f);
					num10 = 1.0f - (f + 1.0f) * 0.5f;
				} else
				{
					float f2 = Mathf.Cos((num9 - 1.0f) * MATHF_PI) * 1.1f;
					f2 = Mathf.Sign(f2) * Mathf.Pow(f2,4.0f);
					f2 = Mathf.Clamp(f2,-1.0f,1.0f);
					num10 = 2.0f - (f2 + 1.0f) * 0.5f;
				}
				num7 = num10;
				num8 = num10;
				num8 = ((num8 < 1.0) ? (Math.Max(num8 - 0.2,0.0) * 1.25) : num8);
				num8 = ((num8 > 1.0) ? Math.Min(num8 * num8,2.0) : num8);
				num8 = Maths::Levelize2(num8);
				data.heightData[i] = (unsigned short)(((double)planet.radius + num7 + 0.1) * 100.0);
			}
		}
	}
};

class PlanetAlgorithm9: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.01;
		double num2 = 0.012;
		double num3 = 0.01;
		double num4 = 3.0;
		double num5 = -0.2;
		double num6 = 0.9;
		double num7 = 0.5;
		double num8 = 2.5;
		double num9 = 0.3;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num10 = dotNet35Random.Next();
		int num11 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num10);
		SimplexNoise simplexNoise2 = SimplexNoise(num11);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain9");

			float custom[3] = {planet.radius,modX,modY};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num12 = data.vertices[i].x * planet.radius;
				double num13 = data.vertices[i].y * planet.radius;
				double num14 = data.vertices[i].z * planet.radius;
				double num15 = 0.0;
				double num16 = 0.0;
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
				num15 = num22 * (1.0 - num28) + x * num28;
				num15 = ((num15 > 0.0) ? (num15 * 0.5) : num15);
				double num29 = simplexNoise2.Noise3DFBM(num12 * num * 1.5,num13 * num2 * 2.0,num14 * num3 * 1.5,6) * num4 + num5;
				num29 = Math.Max(num29 + 1.0,-0.99);
				num29 = ((num29 > 0.0) ? (num29 * 0.25) : num29);
				num16 = Math.Max(val,0.0);
				double num30 = Mathf.Clamp01((float)(num16 - 1.0));
				num16 = ((num16 > 1.0) ? (num30 * num29 * 1.15 + 1.0) : num16);
				num16 = Math.Min(num16,2.0);
				data.heightData[i] = (unsigned short)(((double)planet.radius + num15 + 0.2) * 100.0);
			}
		}
	}
};

class PlanetAlgorithm10: public PlanetAlgorithm
{
private:
	static constexpr int kCircleCount = 10;
	Vector4 ellipses[10] = {};
	double eccentricities[10] = {};
	double heights[10] = {};
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
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.007;
		double num2 = 0.007;
		double num3 = 0.007;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		int num6 = dotNet35Random.Next();
		int num7 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num4);
		SimplexNoise simplexNoise2 = SimplexNoise(num5);
		SimplexNoise simplexNoise3 = SimplexNoise(num6);
		SimplexNoise simplexNoise4 = SimplexNoise(num7);
		int num8 = dotNet35Random.Next();
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain10");

			float custom[61];
			custom[0] = planet.radius;

			for(int i = 0; i < 10; i++)
			{
				VectorLF3 vectorLF = RandomTable::SphericNormal(num8,1.0);
				Vector4 vector = Vector4((float)vectorLF.x,(float)vectorLF.y,(float)vectorLF.z);
				vector.Normalize();
				vector *= planet.radius;
				vector.w = (float)(dotNet35Random.NextDouble() * 10.0 + 40.0);
				custom[4*i+1] = vector.x;
				custom[4*i+2] = vector.y;
				custom[4*i+3] = vector.z;
				custom[4*i+4] = vector.w;
				if(dotNet35Random.NextDouble() > 0.5)
				{
					custom[i+41] = Remap(0.0,1.0,3.0,5.0,dotNet35Random.NextDouble());
				} else
				{
					custom[i+41] = Remap(0.0,1.0,0.2,1.0 / 3.0,dotNet35Random.NextDouble());
				}
				custom[i+51] = Remap(0.0,1.0,1.0,2.0,dotNet35Random.NextDouble());
			}

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer perm_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise3.perm);
			cl::Buffer perm_buffer_4(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise4.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer permMod12_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise3.permMod12);
			cl::Buffer permMod12_buffer_4(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise4.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,perm_buffer_3);
			kernel.setArg(5,perm_buffer_4);
			kernel.setArg(6,permMod12_buffer_1);
			kernel.setArg(7,permMod12_buffer_2);
			kernel.setArg(8,permMod12_buffer_3);
			kernel.setArg(9,permMod12_buffer_4);
			kernel.setArg(10,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < 10; i++)
			{
				VectorLF3 vectorLF = RandomTable::SphericNormal(num8,1.0);
				Vector4 vector = Vector4((float)vectorLF.x,(float)vectorLF.y,(float)vectorLF.z);
				vector.Normalize();
				vector *= planet.radius;
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
			for(int j = 0; j < DATALENGTH; j++)
			{
				double num9 = data.vertices[j].x * planet.radius;
				double num10 = data.vertices[j].y * planet.radius;
				double num11 = data.vertices[j].z * planet.radius;
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
				{
					f = ((!(f > 0.4)) ? (f + x) : (f + num32));
				}
				double a = f * 2.5 - f * num23;
				num19 = Max(a,x * 2.0);
				double num33 = (2.0 - num19) / 2.0;
				num19 -= num16 * 1.2 * num17 * num33;
				if(num19 >= 0.0)
				{
					num19 += (num15 * 0.25 + num18 * 0.6) * num33;
				}
				num19 -= 0.1;
				num20 = num19;
				num20 = Max(num20,-1.0);
				num20 = Math.Abs(num20);
				double num34 = 100.0;
				if(f < 0.4)
				{
					num20 += Remap(-1.0,1.0,-0.2,0.2,simplexNoise.Noise3DFBM(num9 * 2.0 + num34,num10 * 2.0 + num34,num11 * 2.0 + num34,5));
				}
				data.heightData[j] = (unsigned short)(((double)planet.radius + num19 + 0.1) * 100.0);
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
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.007;
		double num2 = 0.007;
		double num3 = 0.007;
		double num4 = 0.002 * modX;
		double num5 = 0.002 * modX * 4.0;
		double num6 = 0.002 * modX;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num7 = dotNet35Random.Next();
		int num8 = dotNet35Random.Next();
		int num9 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num7);
		SimplexNoise simplexNoise2 = SimplexNoise(num8);
		SimplexNoise simplexNoise3 = SimplexNoise(num9);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain11");

			float custom[5] = {planet.radius,num4,num5,num6,modY};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer perm_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise3.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer permMod12_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise3.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,perm_buffer_3);
			kernel.setArg(5,permMod12_buffer_1);
			kernel.setArg(6,permMod12_buffer_2);
			kernel.setArg(7,permMod12_buffer_3);
			kernel.setArg(8,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num10 = data.vertices[i].x * planet.radius;
				double num11 = data.vertices[i].y * planet.radius;
				double num12 = data.vertices[i].z * planet.radius;
				double num13 = 0.0;
				double num14 = 0.0;
				double num15 = simplexNoise2.Noise3DFBM(num10 * num * 4.0,num11 * num2 * 8.0,num12 * num3 * 4.0,3);
				double num16 = 0.6;
				double x = simplexNoise.Noise3DFBM(num10 * num * num16,num11 * num * 1.5 * 2.5,num12 * num * num16,6,0.45,1.8) * 0.95 + num15 * 0.05;
				x = Remap(-1.0,1.0,0.0,1.0,x);
				x = Math.Pow(x,modY);
				x += 1.0;
				x = Maths::Levelize2(x);
				double x2 = simplexNoise3.Noise3DFBM(num10 * num4,num11 * num5,num12 * num6,5,0.55);
				x2 = Remap(-1.0,1.0,0.0,1.0,x2);
				x2 = Math.Pow(x2,0.65);
				num14 = Maths::Levelize3(x2) * x;
				num13 = (num14 - 0.4) * 0.9;
				num13 = Math.Max(-0.3,num13);
				data.heightData[i] = (unsigned short)(((double)planet.radius + num13) * 100.0);
			}
		}
	}

	void GenerateVeins(StarClass& star,PlanetClass& planet,int birthPlanetId,int* veins,int* res) override {
		ThemeProto themeProto = LDB.Select(planet.theme);
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		int birthSeed = dotNet35Random.Next();
		DotNet35Random dotNet35Random2 = DotNet35Random(dotNet35Random.Next());
		PlanetRawData& data = planet.data;
		float num = 2.1f / planet.radius;
		int array[15] = {0};
		float array2[15] = {0};
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
		float p = 1.0f;
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
			break;
		}
		}
		for(int n = 0; n < themeProto.RareVeins.size(); n++)
		{
			int num2 = themeProto.RareVeins[n];
			float num3 = ((star.index == 0) ? themeProto.RareSettings[n * 4] : themeProto.RareSettings[n * 4 + 1]);
			float num4 = themeProto.RareSettings[n * 4 + 2];
			float num5 = themeProto.RareSettings[n * 4 + 3];
			float num6 = num5;
			num3 = 1.0f - Mathf.Pow(1.0f - num3,p);
			num5 = 1.0f - Mathf.Pow(1.0f - num5,p);
			num6 = 1.0f - Mathf.Pow(1.0f - num6,p);
			if(!(dotNet35Random.NextDouble() < (double)num3))
			{
				continue;
			}
			array[num2]++;
			array2[num2] = num5;
			for(int num7 = 1; num7 < 12; num7++)
			{
				if(dotNet35Random.NextDouble() >= (double)num4)
				{
					break;
				}
				array[num2]++;
			}
		}
		bool flag = birthPlanetId == planet.id;
		//if(flag)
		//{
		//	planet.GenBirthPoints(data,birthSeed);
		//}
		veinVectorCount = 0;
		Vector3 birthPoint;
		if(false)
		{

		}
		//if(flag)
		//{
		//	birthPoint = planet.birthPoint;
		//	birthPoint.Normalize();
		//	birthPoint *= 0.75f;
		//}
		else
		{
			birthPoint.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.y = (float)dotNet35Random2.NextDouble() - 0.5f;
			birthPoint.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.Normalize();
			birthPoint *= (float)(dotNet35Random2.NextDouble() * 0.4 + 0.2);
		}
		//if(flag)
		//{
		//	veinVectorTypes[0] = EVeinType::Iron;
		//	veinVectors[0] = planet.birthResourcePoint0;
		//	veinVectorTypes[1] = EVeinType::Copper;
		//	veinVectors[1] = planet.birthResourcePoint1;
		//	veinVectorCount = 2;
		//}
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
					float target_height = data.QueryHeight(target_pos);
					if(target_height < planet.radius || (eVeinType == EVeinType::Oil && target_height < planet.radius + 0.5f) || ((int)eVeinType <= 2 && target_height > planet.radius + 0.7f) || ((eVeinType == EVeinType::Silicium || eVeinType == EVeinType::Titanium) && target_height <= planet.radius + 0.7f))
					{
						continue;
					}
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
		tmp_vecs.clear();
		for(int vein_group_index = 0; vein_group_index < veinVectorCount; vein_group_index++)
		{
			tmp_vecs.clear();
			Vector3 normalized = Vector3::Normalize(veinVectors[vein_group_index]);
			EVeinType eVeinType2 = veinVectorTypes[vein_group_index];
			int vein_point_type = (int)eVeinType2;
			veins[vein_point_type-1]++;
			glm::quat quaternion = glm::rotation(vector3_to_glm(Vector3::up()),vector3_to_glm(normalized));
			Vector3 vector = glm_to_vector3(quaternion * vector3_to_glm(Vector3::right()));
			Vector3 vector2 = glm_to_vector3(quaternion * vector3_to_glm(Vector3::forward()));
			tmp_vecs.push_back(Vector2::zero());
			int vein_point_num = Mathf.RoundToInt(array2[vein_point_type] * (float)dotNet35Random2.Next(20,25));
			if(eVeinType2 == EVeinType::Oil)
			{
				vein_point_num = 1;
			}
			//if(flag && vein_group_index < 2)
			//{
			//	vein_point_num = 6;
			//}
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
			for(int vein_point_index = 0; vein_point_index < tmp_vecs.size(); vein_point_index++)
			{
				Vector3 vector5 = (vector * tmp_vecs[vein_point_index].x + vector2 * tmp_vecs[vein_point_index].y) * num;
				dotNet35Random2.Next();
				Vector3 vein_pos = normalized + vector5;
				//TODO: 这里对油井未对齐！
				//if(vein.type == EVeinType::Oil)
				//{
				//	vein.pos = planet.aux.RawSnap(vein.pos);
				//}
				float num29 = data.QueryHeight(vein_pos);
				if(planet.waterItemId == 0 || num29 >= planet.radius)
				{
					res[vein_point_type-1]++;
				}
			}
		}
		tmp_vecs.clear();
	};
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
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 1.1 * modX;
		double num2 = 0.2;
		double num3 = 8.0;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num4);
		SimplexNoise simplexNoise2 = SimplexNoise(num5);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain12");

			float custom[3] = {planet.radius,num,modY};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,permMod12_buffer_1);
			kernel.setArg(5,permMod12_buffer_2);
			kernel.setArg(6,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num6 = Math.Abs(Math.Asin(data.vertices[i].y)) * 2.0 / Math.PI;
				double num7 = 0.0;
				double num8 = 0.0;
				double num9 = 0.0;
				double num10 = 0.0;
				double num11 = data.vertices[i].x;
				double num12 = (double)data.vertices[i].y * 2.5 * modY;
				double num13 = data.vertices[i].z;
				double num14 = simplexNoise2.Noise3DFBM(num11 * num,num12 * num,num13 * num,3,0.4) * 0.2;
				num9 = simplexNoise.RidgedNoise(num11 * num,num12 * num - num14,num13 * num,6,0.7,2.0,0.8);
				num10 = simplexNoise.Noise3DFBMInitialAmp(num11 * num,num12 * num - num14,num13 * num,6,0.6,2.0,0.7);
				num10 *= num9 + num10;
				num10 = num2 + num3 * num10 * num9;
				double x = num10 + 0.5;
				x = Remap(-8.0,8.0,0.0,1.0,x);
				x = Maths::Clamp01(x);
				x += 0.5;
				x = Math.Pow(x,1.5);
				x -= CurveEvaluate((float)(num6 * 0.9));
				num8 = x * 2.0;
				num7 = Maths::Clamp(num8,0.0,2.0);
				num7 = num7 * 1.1 - 0.2;
				data.heightData[i] = (unsigned short)(((double)planet.radius + num7) * 100.0);
			}
		}
	}

	void GenerateVeins(StarClass& star,PlanetClass& planet,int birthPlanetId,int* veins,int* res) override {
		ThemeProto themeProto = LDB.Select(planet.theme);
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		int birthSeed = dotNet35Random.Next();
		DotNet35Random dotNet35Random2 = DotNet35Random(dotNet35Random.Next());
		PlanetRawData& data = planet.data;
		float num = 2.1f / planet.radius;
		int array[15] = {0};
		float array2[15] = {0};
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
		float p = 1.0f;
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
			break;
		}
		}
		for(int n = 0; n < themeProto.RareVeins.size(); n++)
		{
			int num2 = themeProto.RareVeins[n];
			float num3 = ((star.index == 0) ? themeProto.RareSettings[n * 4] : themeProto.RareSettings[n * 4 + 1]);
			float num4 = themeProto.RareSettings[n * 4 + 2];
			float num5 = themeProto.RareSettings[n * 4 + 3];
			float num6 = num5;
			num3 = 1.0f - Mathf.Pow(1.0f - num3,p);
			num5 = 1.0f - Mathf.Pow(1.0f - num5,p);
			num6 = 1.0f - Mathf.Pow(1.0f - num6,p);
			if(!(dotNet35Random.NextDouble() < (double)num3))
			{
				continue;
			}
			array[num2]++;
			array2[num2] = num5;
			for(int num7 = 1; num7 < 12; num7++)
			{
				if(dotNet35Random.NextDouble() >= (double)num4)
				{
					break;
				}
				array[num2]++;
			}
		}
		bool flag = birthPlanetId == planet.id;
		//if(flag)
		//{
		//	planet.GenBirthPoints(data,birthSeed);
		//}
		veinVectorCount = 0;
		Vector3 birthPoint;
		if(false)
		{

		}
		//if(flag)
		//{
		//	birthPoint = planet.birthPoint;
		//	birthPoint.Normalize();
		//	birthPoint *= 0.75f;
		//}
		else
		{
			birthPoint.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.y = (float)dotNet35Random2.NextDouble() - 0.5f;
			birthPoint.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.Normalize();
			birthPoint *= (float)(dotNet35Random2.NextDouble() * 0.4 + 0.2);
		}
		//if(flag)
		//{
		//	veinVectorTypes[0] = EVeinType::Iron;
		//	veinVectors[0] = planet.birthResourcePoint0;
		//	veinVectorTypes[1] = EVeinType::Copper;
		//	veinVectors[1] = planet.birthResourcePoint1;
		//	veinVectorCount = 2;
		//}
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
					float target_height = data.QueryHeight(target_pos);
					if(target_height < planet.radius || (eVeinType == EVeinType::Oil && target_height < planet.radius + 0.5f) || (eVeinType == EVeinType::Fireice && target_height < planet.radius + 1.2f))
					{
						continue;
					}
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
		tmp_vecs.clear();
		for(int vein_group_index = 0; vein_group_index < veinVectorCount; vein_group_index++)
		{
			tmp_vecs.clear();
			Vector3 normalized = Vector3::Normalize(veinVectors[vein_group_index]);
			EVeinType eVeinType2 = veinVectorTypes[vein_group_index];
			int vein_point_type = (int)eVeinType2;
			veins[vein_point_type-1]++;
			glm::quat quaternion = glm::rotation(vector3_to_glm(Vector3::up()),vector3_to_glm(normalized));
			Vector3 vector = glm_to_vector3(quaternion * vector3_to_glm(Vector3::right()));
			Vector3 vector2 = glm_to_vector3(quaternion * vector3_to_glm(Vector3::forward()));
			tmp_vecs.push_back(Vector2::zero());
			int vein_point_num = Mathf.RoundToInt(array2[vein_point_type] * (float)dotNet35Random2.Next(20,25));
			if(eVeinType2 == EVeinType::Oil)
			{
				vein_point_num = 1;
			}
			//if(flag && vein_group_index < 2)
			//{
			//	vein_point_num = 6;
			//}
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
			for(int vein_point_index = 0; vein_point_index < tmp_vecs.size(); vein_point_index++)
			{
				Vector3 vector5 = (vector * tmp_vecs[vein_point_index].x + vector2 * tmp_vecs[vein_point_index].y) * num;
				dotNet35Random2.Next();
				Vector3 vein_pos = normalized + vector5;
				//TODO: 这里对油井未对齐！
				//if(vein.type == EVeinType::Oil)
				//{
				//	vein.pos = planet.aux.RawSnap(vein.pos);
				//}
				float num29 = data.QueryHeight(vein_pos);
				if(planet.waterItemId == 0 || num29 >= planet.radius)
				{
					res[vein_point_type-1]++;
				}
			}
		}
		tmp_vecs.clear();
	};
};

class PlanetAlgorithm13: public PlanetAlgorithm
{
private:
	double Remap(double sourceMin,double sourceMax,double targetMin,double targetMax,double x)
	{
		return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
	}
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.007 * modX;
		double num2 = 0.007 * modX;
		double num3 = 0.007 * modX;
		SimplexNoise simplexNoise = SimplexNoise(DotNet35Random(planet.seed).Next());
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain13");

			float custom[5] = {planet.radius,num,num2,num3,modY};

			cl::Buffer custom_buffer(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(custom),custom);
			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,custom_buffer);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,permMod12_buffer_1);
			kernel.setArg(4,heightData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num4 = data.vertices[i].x * planet.radius;
				double num5 = data.vertices[i].y * planet.radius;
				double num6 = data.vertices[i].z * planet.radius;
				double num7 = 0.0;
				double num8 = 0.0;
				double x = Remap(-1.0,1.0,0.0,1.0,simplexNoise.Noise3DFBM(num4 * num,num5 * num2,num6 * num3,6));
				x = Math.Pow(x,modY) * 3.0625;
				x = Remap(0.0,2.0,0.0,4.0,x);
				if(x < 1.0)
				{
					x = Math.Pow(x,2.0);
				}
				x -= 0.2;
				num8 = Math.Min(x,4.0);
				Math.Max(1.0 - Math.Abs(1.0 - num8),0.0);
				if(num8 > 2.0)
				{
					num8 = ((!(num8 > 3.0)) ? (2.0 - 1.0 * (num8 - 2.0)) : ((!(num8 > 3.5)) ? 1.0 : (1.0 + 2.0 * (num8 - 3.5))));
				}
				num7 = num8;
				data.heightData[i] = (unsigned short)(((double)planet.radius + num7 + 0.1) * 100.0);
			}
		}
	}

	void GenerateVeins(StarClass& star,PlanetClass& planet,int birthPlanetId,int* veins,int* res) override {
		ThemeProto themeProto = LDB.Select(planet.theme);
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		dotNet35Random.Next();
		int birthSeed = dotNet35Random.Next();
		DotNet35Random dotNet35Random2 = DotNet35Random(dotNet35Random.Next());
		PlanetRawData& data = planet.data;
		float num = 2.1f / planet.radius;
		int array[15] = {0};
		float array2[15] = {0};
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
		float p = 1.0f;
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
			break;
		}
		}
		for(int n = 0; n < themeProto.RareVeins.size(); n++)
		{
			int num2 = themeProto.RareVeins[n];
			float num3 = ((star.index == 0) ? themeProto.RareSettings[n * 4] : themeProto.RareSettings[n * 4 + 1]);
			float num4 = themeProto.RareSettings[n * 4 + 2];
			float num5 = themeProto.RareSettings[n * 4 + 3];
			float num6 = num5;
			num3 = 1.0f - Mathf.Pow(1.0f - num3,p);
			num5 = 1.0f - Mathf.Pow(1.0f - num5,p);
			num6 = 1.0f - Mathf.Pow(1.0f - num6,p);
			if(!(dotNet35Random.NextDouble() < (double)num3))
			{
				continue;
			}
			array[num2]++;
			array2[num2] = num5;
			for(int num7 = 1; num7 < 12; num7++)
			{
				if(dotNet35Random.NextDouble() >= (double)num4)
				{
					break;
				}
				array[num2]++;
			}
		}
		bool flag = birthPlanetId == planet.id;
		//if(flag)
		//{
		//	planet.GenBirthPoints(data,birthSeed);
		//}
		veinVectorCount = 0;
		Vector3 birthPoint;
		if(false)
		{

		}
		//if(flag)
		//{
		//	birthPoint = planet.birthPoint;
		//	birthPoint.Normalize();
		//	birthPoint *= 0.75f;
		//}
		else
		{
			birthPoint.x = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.y = (float)dotNet35Random2.NextDouble() - 0.5f;
			birthPoint.z = (float)dotNet35Random2.NextDouble() * 2.0f - 1.0f;
			birthPoint.Normalize();
			birthPoint *= (float)(dotNet35Random2.NextDouble() * 0.4 + 0.2);
		}
		//if(flag)
		//{
		//	veinVectorTypes[0] = EVeinType::Iron;
		//	veinVectors[0] = planet.birthResourcePoint0;
		//	veinVectorTypes[1] = EVeinType::Copper;
		//	veinVectors[1] = planet.birthResourcePoint1;
		//	veinVectorCount = 2;
		//}
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
					float target_height = data.QueryHeight(target_pos);
					if(target_height < planet.radius || (eVeinType == EVeinType::Oil && target_height < planet.radius + 0.5f) || ((int)eVeinType <= 4 && target_height > planet.radius + 0.7f))
					{
						continue;
					}
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
		tmp_vecs.clear();
		for(int vein_group_index = 0; vein_group_index < veinVectorCount; vein_group_index++)
		{
			tmp_vecs.clear();
			Vector3 normalized = Vector3::Normalize(veinVectors[vein_group_index]);
			EVeinType eVeinType2 = veinVectorTypes[vein_group_index];
			int vein_point_type = (int)eVeinType2;
			veins[vein_point_type-1]++;
			glm::quat quaternion = glm::rotation(vector3_to_glm(Vector3::up()),vector3_to_glm(normalized));
			Vector3 vector = glm_to_vector3(quaternion * vector3_to_glm(Vector3::right()));
			Vector3 vector2 = glm_to_vector3(quaternion * vector3_to_glm(Vector3::forward()));
			tmp_vecs.push_back(Vector2::zero());
			int vein_point_num = Mathf.RoundToInt(array2[vein_point_type] * (float)dotNet35Random2.Next(20,25));
			if(eVeinType2 == EVeinType::Oil)
			{
				vein_point_num = 1;
			}
			//if(flag && vein_group_index < 2)
			//{
			//	vein_point_num = 6;
			//}
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
			for(int vein_point_index = 0; vein_point_index < tmp_vecs.size(); vein_point_index++)
			{
				Vector3 vector5 = (vector * tmp_vecs[vein_point_index].x + vector2 * tmp_vecs[vein_point_index].y) * num;
				dotNet35Random2.Next();
				Vector3 vein_pos = normalized + vector5;
				//TODO: 这里对油井未对齐！
				//if(vein.type == EVeinType::Oil)
				//{
				//	vein.pos = planet.aux.RawSnap(vein.pos);
				//}
				float num29 = data.QueryHeight(vein_pos);
				if(planet.waterItemId == 0 || num29 >= planet.radius)
				{
					res[vein_point_type-1]++;
				}
			}
		}
		tmp_vecs.clear();
	};
};

class PlanetAlgorithm14: public PlanetAlgorithm
{
public:
	void GenerateTerrain(PlanetClass& planet,double modX,double modY) override {
		double num = 0.007;
		double num2 = 0.007;
		double num3 = 0.007;
		DotNet35Random dotNet35Random = DotNet35Random(planet.seed);
		int num4 = dotNet35Random.Next();
		int num5 = dotNet35Random.Next();
		int num6 = dotNet35Random.Next();
		int num7 = dotNet35Random.Next();
		SimplexNoise simplexNoise = SimplexNoise(num4);
		SimplexNoise simplexNoise2 = SimplexNoise(num5);
		SimplexNoise simplexNoise3 = SimplexNoise(num6);
		SimplexNoise simplexNoise4 = SimplexNoise(num7);
		PlanetRawData& data = planet.data;
		data.heightData.resize(DATALENGTH);
		//data.debugData.resize(DATALENGTH);
		if(OpenCLManager::SUPPORT_GPU && OpenCLManager::SUPPORT_DOUBLE) {
			cl::Kernel kernel(OpenCLManager::program,"GenerateTerrain14");

			cl::Buffer perm_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.perm);
			cl::Buffer perm_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.perm);
			cl::Buffer perm_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise3.perm);
			cl::Buffer perm_buffer_4(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise4.perm);
			cl::Buffer permMod12_buffer_1(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise.permMod12);
			cl::Buffer permMod12_buffer_2(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise2.permMod12);
			cl::Buffer permMod12_buffer_3(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise3.permMod12);
			cl::Buffer permMod12_buffer_4(OpenCLManager::context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(short) * PERM_LENGTH,simplexNoise4.permMod12);
			cl::Buffer heightData_buffer(OpenCLManager::context,CL_MEM_WRITE_ONLY,sizeof(unsigned short) * DATALENGTH);

			kernel.setArg(0,OpenCLManager::vertices_buffer);
			kernel.setArg(1,sizeof(float),&planet.radius);
			kernel.setArg(2,perm_buffer_1);
			kernel.setArg(3,perm_buffer_2);
			kernel.setArg(4,perm_buffer_3);
			kernel.setArg(5,perm_buffer_4);
			kernel.setArg(6,permMod12_buffer_1);
			kernel.setArg(7,permMod12_buffer_2);
			kernel.setArg(8,permMod12_buffer_3);
			kernel.setArg(9,permMod12_buffer_4);
			kernel.setArg(10,heightData_buffer);
			//kernel.setArg(11,OpenCLManager::debugData_buffer);

			int local_size = OpenCLManager::local_size;
			int global_size = (int)ceil(161604.0/local_size) * local_size;
			cl_int err = OpenCLManager::queue.enqueueNDRangeKernel(kernel,cl::NullRange,{(size_t)global_size},{(size_t)local_size});
			OpenCLManager::queue.finish();
			if(err != CL_SUCCESS){
				std::cerr << "Kernel execution failed with error code: " << err << std::endl;
				throw std::runtime_error("Kernel execution failed");
			}

			OpenCLManager::queue.enqueueReadBuffer(heightData_buffer,CL_TRUE,0,
						  sizeof(unsigned short) * data.heightData.size(),data.heightData.data());
			//OpenCLManager::queue.enqueueReadBuffer(OpenCLManager::debugData_buffer,CL_TRUE,0,
			//			  sizeof(float) * data.debugData.size(),data.debugData.data());
		} else {
			for(int i = 0; i < DATALENGTH; i++)
			{
				double num8 = data.vertices[i].x * planet.radius;
				double num9 = data.vertices[i].y * planet.radius;
				double num10 = data.vertices[i].z * planet.radius;
				double num11 = Maths::Levelize(num8 * 0.007 / 2.0);
				double num12 = Maths::Levelize(num9 * 0.007 / 2.0);
				double num13 = Maths::Levelize(num10 * 0.007 / 2.0);
				num11 += simplexNoise3.Noise(num8 * 0.05,num9 * 0.05,num10 * 0.05) * 0.04;
				num12 += simplexNoise3.Noise(num9 * 0.05,num10 * 0.05,num8 * 0.05) * 0.04;
				num13 += simplexNoise3.Noise(num10 * 0.05,num8 * 0.05,num9 * 0.05) * 0.04;
				double num14 = Math.Abs(simplexNoise4.Noise(num11,num12,num13));
				double num15 = (0.12 - num14) * 10.0;
				num15 = ((!(num15 > 0.0)) ? 0.0 : ((num15 > 1.0) ? 1.0 : num15));
				num15 *= num15;
				double num16 = (simplexNoise3.Noise3DFBM(num9 * 0.005,num10 * 0.005,num8 * 0.005,4) + 0.22) * 5.0;
				num16 = ((!(num16 > 0.0)) ? 0.0 : ((num16 > 1.0) ? 1.0 : num16));
				Math.Abs(simplexNoise4.Noise3DFBM(num11 * 1.5,num12 * 1.5,num13 * 1.5,2));
				num8 += Math.Sin(num9 * 0.15) * 3.0;
				num9 += Math.Sin(num10 * 0.15) * 3.0;
				num10 += Math.Sin(num8 * 0.15) * 3.0;
				double num17 = 0.0;
				double num18 = 0.0;
				double num19 = simplexNoise.Noise3DFBM(num8 * num * 1.0,num9 * num2 * 1.1,num10 * num3 * 1.0,6,0.5,1.8);
				double num20 = simplexNoise2.Noise3DFBM(num8 * num * 1.3 + 0.5,num9 * num2 * 2.8 + 0.2,num10 * num3 * 1.3 + 0.7,3) * 2.0;
				double num21 = simplexNoise2.Noise3DFBM(num8 * num * 6.0,num9 * num2 * 12.0,num10 * num3 * 6.0,2) * 2.0;
				double num22 = simplexNoise2.Noise3DFBM(num8 * num * 0.8,num9 * num2 * 0.8,num10 * num3 * 0.8,2) * 2.0;
				double num23 = num19 * 2.0 + 0.92;
				double num24 = num20 * (double)Mathf.Abs((float)num22 + 0.5f);
				num23 += (double)Mathf.Clamp01((float)(num24 - 0.35) * 1.0f);
				if(num23 < 0.0)
				{
					num23 = 0.0;
				}
				double num25 = num23;
				num25 = Maths::Levelize2(num23);
				if(num25 > 0.0)
				{
					num25 = Maths::Levelize2(num23);
					num25 = Maths::Levelize4(num25);
				}
				double num26 = ((!(num25 > 0.0)) ? ((double)Mathf.Lerp(-4.0f,0.0f,(float)num25 + 1.0f)) : ((!(num25 > 1.0)) ? ((double)Mathf.Lerp(0.0f,0.3f,(float)num25) + num21 * 0.1) : ((num25 > 2.0) ? ((double)Mathf.Lerp(1.4f,2.7f,(float)num25 - 2.0f) + num21 * 0.12) : ((double)Mathf.Lerp(0.3f,1.4f,(float)num25 - 1.0f) + num21 * 0.12))));
				if(num23 < 0.0)
				{
					num23 *= 2.0;
				}
				if(num23 < 1.0)
				{
					num23 = Maths::Levelize(num23);
				}
				num17 -= num15 * 1.2 * num16;
				//data.debugData[i] = (float)(num25 <= 0.0) + (float)(num25 <= 1.0) + (float)(num25 > 2.0);
				if(num17 >= 0.0)
				{
					num17 = num26;
				}
				num17 -= 0.1;
				num18 = Mathf.Abs((float)num23);
				double x = Mathf.Clamp01((float)((0.0 - num17 + 2.0) / 2.5));
				x = Math.Pow(x,10.0);
				num18 = (1.0 - x) * num18 + x * 2.0;
				num18 = ((!(num18 > 0.0)) ? 0.0 : ((num18 > 2.0) ? 2.0 : num18));
				num18 += ((num18 > 1.8) ? ((0.0 - num21) * 0.8) : (num21 * 0.2)) * (1.0 - x);
				double num27 = -0.3 - num17;
				if(num27 > 0.0)
				{
					double num28 = simplexNoise2.Noise(num8 * 0.16,num9 * 0.16,num10 * 0.16) - 1.0;
					num27 = ((num27 > 1.0) ? 1.0 : num27);
					num27 = (3.0 - num27 - num27) * num27 * num27;
					num17 = -0.3 - num27 * 10.0 + num27 * num27 * num27 * num27 * num28 * 0.5;
				}
				data.heightData[i] = (unsigned short)(((double)planet.radius + num17 + 0.2) * 100.0);
			}
		}
	}
};

std::unique_ptr<PlanetAlgorithm> PlanetAlgorithmManager(int algoId);
