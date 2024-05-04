#include "StringUtils.hpp"

std::vector<std::string> StringUtils::splitByDelimiter(const std::string &input, const std::string &delimiter)
{
	std::vector<std::string> result;
	size_t pos = 0;
	size_t prev = 0;
	while ((pos = input.find(delimiter, prev)) != std::string::npos)
	{
		if (pos > prev)
		{
			result.push_back(input.substr(prev, pos - prev));
		}
		prev = pos + delimiter.length(); // Move past delimiter
	}
	// Add the last substring if it exists
	if (prev < input.length())
	{
		result.push_back(input.substr(prev, std::string::npos));
	}
	return result;
}

std::string StringUtils::trim(const std::string &str)
{
	size_t first = 0;
	size_t last = str.length() - 1;

	// Find the first non-whitespace character
	while (first < str.length() && std::isspace(static_cast<unsigned char>(str[first])))
	{
		++first;
	}

	// Find the last non-whitespace character
	while (last > first && std::isspace(static_cast<unsigned char>(str[last])))
	{
		--last;
	}

	// If the string is all whitespace, return an empty string
	if (first > last)
	{
		return "";
	}

	// Return the substring from the first non-whitespace character to the last non-whitespace character
	return str.substr(first, (last - first + 1));
}

bool StringUtils::isDigitsOnly(const std::string &str)
{
	return std::all_of(
		str.begin(), str.end(), [](unsigned char c)
		{ return std::isdigit(c); });
}

size_t StringUtils::strToSizeT(const std::string &str)
{
	if (str.empty() || !isDigitsOnly(str))
	{
		throw std::invalid_argument("Invalid string argument");
	}
	try
	{
		unsigned long long value = std::stoull(str);
		if (value > std::numeric_limits<size_t>::max())
		{
			throw std::out_of_range("Value out of range for size_t");
		}
		return static_cast<size_t>(value);
	}
	catch (const std::exception &e)
	{
		throw std::invalid_argument("Invalid string argument");
	}
}

void StringUtils::replaceAll(std::string &str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

std::string StringUtils::queryStr(const std::string &str)
{
	std::string queryStr = "";
	std::size_t pos = str.find("?");
	if (pos != std::string::npos)
	{
		queryStr = str.substr(pos + 1);
	}
	return queryStr;
}

std::string StringUtils::extractPathPreQuery(const std::string &url)
{
	std::size_t pos = url.find('?');
	if (pos != std::string::npos)
	{
		// Return the part of the string before the '?'
		return url.substr(0, pos);
	}
	else
	{
		// Return the whole string if there is no '?'
		return url;
	}
}

std::unordered_map<std::string, std::string> StringUtils::parseQueryString(const std::string &queryString)
{
	std::unordered_map<std::string, std::string> queryMap;
	std::stringstream ss(queryString);
	std::string item;

	while (std::getline(ss, item, '&'))
	{
		std::stringstream itemStream(item);
		std::string key, value;
		std::getline(itemStream, key, '=');
		std::getline(itemStream, value, '=');
		queryMap[key] = value;
	}

	return queryMap;
}

std::string StringUtils::trimChar(const std::string &str, char ch)
{
	size_t start = str.find_first_not_of(ch);
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(ch);
	if (end == std::string::npos)
		return "";
	std::string res = str.substr(start, end - start + 1);
	return res;
}

void StringUtils::replaceFirstOccurrence(std::string &str, const std::string &toReplace, const std::string &replaceWith)
{
	size_t pos = str.find(toReplace);
	if (pos != std::string::npos)
	{
		// Replace the first occurrence
		str.replace(pos, toReplace.length(), replaceWith);
	}
}
