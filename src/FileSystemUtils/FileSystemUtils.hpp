#ifndef FILE_SYSTEM_UTILS_HPP
#define FILE_SYSTEM_UTILS_HPP

#include "../StringUtils/StringUtils.hpp"
#include "../defines.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <vector>

namespace FileSystemUtils
{
	bool pathExists(const std::string &target);
	bool pathExistsAndAccessible(const std::string &target);
	bool isDir(const std::string &target);
	bool isFile(const std::string &target);
	void createDirectory(const std::string &path);
	void saveFile(std::string savePath, std::string fileName, std::vector<std::byte> body);
}

#endif
