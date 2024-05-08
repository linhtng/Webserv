#ifndef FILE_SYSTEM_UTILS_HPP
#define FILE_SYSTEM_UTILS_HPP

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <vector>

#include "StringUtils.hpp"
#include "../defines.hpp"

namespace FileSystemUtils
{
	bool pathExists(std::string target);
	bool pathExistsAndAccessible(const std::string &target);
	bool isDir(const std::string &target);
	bool isFile(const std::string &target);
	void createDirectory(const std::string &path);
	void saveFile(std::string savePath, std::string fileName, std::vector<std::byte> body);
	void deleteFile(const std::string &path);
}

#endif
