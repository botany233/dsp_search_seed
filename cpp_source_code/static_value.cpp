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
cl::Device OpenCLManager::device;
cl::Program OpenCLManager::program;
cl::Buffer OpenCLManager::vertices_buffer;
size_t OpenCLManager::cfg_version;
mutex OpenCLManager::lock;
int OpenCLManager::max_worker = 8;
int OpenCLManager::cur_worker = 0;
