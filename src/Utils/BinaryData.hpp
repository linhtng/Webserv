#ifndef BINARY_DATA_HPP
#define BINARY_DATA_HPP

#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>

#include "../HttpMessage/HttpMessage.hpp"
#include "StringUtils.hpp"
#include "Logger.hpp"
#include "../defines.hpp"

namespace BinaryData
{
	std::vector<std::byte> getErrorPage(HttpStatusCode statusCode);
	std::vector<std::byte> getDirectoryListingPage(std::string locationPath, std::string actualLocationPath, std::string pathAfterLocation);
	std::vector<std::byte> getFileData(std::string path);
	std::vector<std::byte> strToVectorByte(std::string const &str);
};

#endif