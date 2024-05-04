#include "FileSystemUtils.hpp"

bool FileSystemUtils::pathExists(const std::string &target)
{
	struct stat path_stat;
	return stat(target.c_str(), &path_stat) == 0;
}

bool FileSystemUtils::pathExistsAndAccessible(const std::string &target)
{
	return access(target.c_str(), F_OK | R_OK | X_OK) == 0;
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

bool FileSystemUtils::isFile(const std::string &target)
{
	struct stat path_stat;
	if (stat(target.c_str(), &path_stat) == -1)
	{
		return false;
	}
	return S_ISREG(path_stat.st_mode);
}

void FileSystemUtils::createDirectory(const std::string &path)
{
	// Attempt to create the directory
	if (mkdir(path.c_str(), 0755) == -1)
	{
		// Check if the error was because the directory already exists
		if (errno != EEXIST)
		{
			throw std::runtime_error("Failed to create directory: " + path);
		}
		// If the directory already exists, do nothing
	}
}

void FileSystemUtils::saveFile(std::string savePath, std::string fileName, std::vector<std::byte> body)
{
	createDirectory(savePath);
	std::string fullPath = StringUtils::joinPath(savePath, fileName);
	std::ofstream fileStream(fullPath, std::ios::binary);
	if (!fileStream)
	{
		throw std::runtime_error("Failed to open file: " + fullPath);
	}
	fileStream.write(reinterpret_cast<const char *>(body.data()), body.size());
	fileStream.close();
}