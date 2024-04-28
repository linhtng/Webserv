#ifndef FILE_SYSTEM_UTILS_HPP
#define FILE_SYSTEM_UTILS_HPP

#include "../defines.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

namespace FileSystemUtils
{
	bool pathExists(const std::string &target);
	bool isDir(const std::string &target);
	bool isFile(const std::string &target);
}

#endif
