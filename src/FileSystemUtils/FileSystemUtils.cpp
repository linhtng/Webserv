#include "FileSystemUtils.hpp"

bool FileSystemUtils::pathExists(const std::string &target)
{
	struct stat path_stat;
	return stat(target.c_str(), &path_stat) == 0;
}

bool FileSystemUtils::isDir(const std::string &target)
{
	struct stat path_stat;
	if (stat(target.c_str(), &path_stat) == -1)
	{
		// stat failed, which could be because the file or directory does not exist
		// or for some other reason (e.g., permission issues).
		// In this case, we assume it's not a directory.
		return false;
	}
	return S_ISDIR(path_stat.st_mode);
}

#include <iostream>

bool FileSystemUtils::isFile(const std::string &target)
{
	std::cout << "Checking if " << target << " is a file" << std::endl;
	struct stat path_stat;
	if (stat(target.c_str(), &path_stat) == -1)
	{
		return false;
	}
	return S_ISREG(path_stat.st_mode);
}
