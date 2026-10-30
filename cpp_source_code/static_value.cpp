#include <vector>
#include <mutex>
#include <cstdint>

#include "LDB.hpp"
LDB_CLASS LDB;

#include "NameGen.hpp"
NameGen_t NameGen;

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
std::vector<std::string> OpenCLManager::devices_info;
cl::Context OpenCLManager::context;
cl::Device OpenCLManager::device;
cl::Program OpenCLManager::program;
cl::Buffer OpenCLManager::vertices_buffer;
std::size_t OpenCLManager::cfg_version;
std::mutex OpenCLManager::lock;
int OpenCLManager::max_worker = 8;
int OpenCLManager::cur_worker = 0;
