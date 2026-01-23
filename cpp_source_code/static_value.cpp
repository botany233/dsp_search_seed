#include <vector>

#include "LDB.hpp"
LDB_CLASS LDB;

#include "PlanetRawData.hpp"
Vector3 PlanetRawData::vertices[161604];
int PlanetRawData::indexMap[60000];

#include <CL/opencl.hpp>
#include "PlanetAlgorithm.hpp"

bool OpenCLManager::SUPPORT_GPU;
bool OpenCLManager::SUPPORT_DOUBLE;
int OpenCLManager::local_size;
int OpenCLManager::device_id;
std::vector<cl::Device> OpenCLManager::devices;
std::vector<string> OpenCLManager::devices_info;
cl::Context OpenCLManager::context;
cl::CommandQueue OpenCLManager::queue;
cl::Program OpenCLManager::program;
cl::Buffer OpenCLManager::vertices_buffer;
