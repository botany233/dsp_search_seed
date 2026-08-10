#include "PlanetAlgorithm.hpp"

bool OpenCLManager::set_device_id(int input_device_id) {
	lock_guard<mutex> lck(lock);
	cfg_version++;

	devices.clear();
	devices_info.clear();
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	for(const cl::Platform& plat : platforms) {
		std::vector<cl::Device> devs;
		plat.getDevices(CL_DEVICE_TYPE_GPU,&devs);
		std::string plat_name = plat.getInfo<CL_PLATFORM_NAME>();
		for(const cl::Device& dev : devs) {
			bool support_double = false;
			try {
				std::string extensions = dev.getInfo<CL_DEVICE_EXTENSIONS>();

				// 检查cl_khr_fp64扩展
				if(extensions.find("cl_khr_fp64") != std::string::npos) {
					support_double = true;
				}

				// 检查cl_amd_fp64扩展（AMD特有）
				if(extensions.find("cl_amd_fp64") != std::string::npos) {
					support_double = true;
				}

				// 直接尝试获取double精度信息
				try {
					cl_device_fp_config doubleConfig = dev.getInfo<CL_DEVICE_DOUBLE_FP_CONFIG>();
					if(doubleConfig != 0) {
						support_double = true;
					}
				} catch(...) {
					// 如果获取double配置失败，继续检查其他方式
				}
			} catch(...) {
				// 扩展信息获取失败时的处理
			}
			if(!support_double)
				continue;

			devices.push_back(dev);
			std::string dev_name = dev.getInfo<CL_DEVICE_NAME>();
			devices_info.push_back(plat_name + " " + dev_name);
		}
	}
	if(input_device_id < 0) {
		SUPPORT_GPU = false;
		return true;
	}
	if(devices.size() <= input_device_id) {
		SUPPORT_GPU = false;
		return false;
	}

	device_id = input_device_id;
	device = devices[device_id];

	// 创建上下文和命令队列
	context = cl::Context(device);

	// 创建程序
	cl::Program::Sources sources;
	AddSources(sources,"assets/generate_terrain_double.cl");
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

	vector<float> vertices(LAND_DATALENGTH*3);
	for(int i=0;i<LAND_DATALENGTH;i++) {
		int index = PlanetAlgorithm::landIndex[i];
		vertices[i*3] = PlanetAlgorithm::vertices[index].x;
		vertices[i*3+1] = PlanetAlgorithm::vertices[index].y;
		vertices[i*3+2] = PlanetAlgorithm::vertices[index].z;
	}
	vertices_buffer = cl::Buffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float) * vertices.size(),vertices.data());

	SUPPORT_GPU = true;
	return true;
}
