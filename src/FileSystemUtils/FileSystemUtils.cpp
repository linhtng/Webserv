#include "FileSystemUtils.hpp"

bool FileSystemUtils::isDir(const std::string &target)
{
	struct stat path_stat;
	stat(target.c_str(), &path_stat);
	return S_ISDIR(path_stat.st_mode);
}
