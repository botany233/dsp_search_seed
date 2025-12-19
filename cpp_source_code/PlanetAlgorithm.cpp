#include <memory>
#include <string>
#include <CL/opencl.hpp>

#include "PlanetRawData.hpp"
#include "PlanetAlgorithm.hpp"

using namespace std;

bool OpenCLManager::SUPPORT_GPU;
bool OpenCLManager::SUPPORT_DOUBLE;
int OpenCLManager::local_size;
int OpenCLManager::device_id;
vector<cl::Device> OpenCLManager::devices;
vector<string> OpenCLManager::devices_info;
cl::Context OpenCLManager::context;
cl::CommandQueue OpenCLManager::queue;
cl::Program OpenCLManager::program;
cl::Buffer OpenCLManager::vertices_buffer;
//cl::Buffer OpenCLManager::custom_buffer;
//cl::Buffer OpenCLManager::perm_buffer_1;
//cl::Buffer OpenCLManager::perm_buffer_2;
//cl::Buffer OpenCLManager::perm_buffer_3;
//cl::Buffer OpenCLManager::perm_buffer_4;
//cl::Buffer OpenCLManager::permMod12_buffer_1;
//cl::Buffer OpenCLManager::permMod12_buffer_2;
//cl::Buffer OpenCLManager::permMod12_buffer_3;
//cl::Buffer OpenCLManager::permMod12_buffer_4;
//cl::Buffer OpenCLManager::heightData_buffer;
//cl::Buffer OpenCLManager::debugData_buffer;

std::unique_ptr<PlanetAlgorithm> PlanetAlgorithmManager(int algoId)
{
	switch(algoId) {
	case 1:
		return std::make_unique<PlanetAlgorithm1>();
	case 2:
		return std::make_unique<PlanetAlgorithm2>();
	case 3:
		return std::make_unique<PlanetAlgorithm3>();
	case 4:
		return std::make_unique<PlanetAlgorithm4>();
	case 5:
		return std::make_unique<PlanetAlgorithm5>();
	case 6:
		return std::make_unique<PlanetAlgorithm6>();
	case 7:
		return std::make_unique<PlanetAlgorithm7>();
	case 8:
		return std::make_unique<PlanetAlgorithm8>();
	case 9:
		return std::make_unique<PlanetAlgorithm9>();
	case 10:
		return std::make_unique<PlanetAlgorithm10>();
	case 11:
		return std::make_unique<PlanetAlgorithm11>();
	case 12:
		return std::make_unique<PlanetAlgorithm12>();
	case 13:
		return std::make_unique<PlanetAlgorithm13>();
	case 14:
		return std::make_unique<PlanetAlgorithm14>();
	default:
		return std::make_unique<PlanetAlgorithm0>();
	}
}
