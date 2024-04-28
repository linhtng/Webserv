#ifndef DIRECTORY_LISTING_PAGE_HPP
#define DIRECTORY_LISTING_PAGE_HPP

#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>

#include "../StringUtils/StringUtils.hpp"

namespace DirectoryListingPage
{
	std::vector<std::byte> getDirectoryListingPage(std::string path);
};

#endif