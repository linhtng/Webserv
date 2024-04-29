#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <limits>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include "../defines.hpp"

namespace StringUtils
{
	size_t strToSizeT(const std::string &str);
	std::vector<std::string> splitByDelimiter(const std::string &input, const std::string &delimiter);
	std::string trim(const std::string &str);
	bool isDigitsOnly(const std::string &str);
	void replaceAll(std::string &str, const std::string &from, const std::string &to);
	std::unordered_map<std::string, std::string> parseQueryString(const std::string &queryString);
	std::string trimChar(const std::string &str, char ch);
}

#endif
