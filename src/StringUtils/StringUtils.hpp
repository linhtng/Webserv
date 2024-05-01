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

	// base case for recursive joinPath
	/* std::string joinPath()
	{
		return "";
	} */

	/* template <typename T, typename... Args>
	inline std::string joinPath(T &&head, Args &&...tail); */

	template <typename T, typename... Args>
	std::string joinPath(T &&head, Args &&...tail)
	{
		std::string trimmedHead = trimChar(std::forward<T>(head), '/');
		if constexpr (sizeof...(Args) == 0)
		{
			// std::cout << "NO ARGS" << std::endl;
			return trimmedHead;
		}
		else
		{
			std::string concatenatedTail = joinPath(std::forward<Args>(tail)...);
			if (concatenatedTail.empty())
			{
				return trimmedHead;
			}
			else
			{
				return trimmedHead + '/' + concatenatedTail;
			}
		}
	}
}

#endif
