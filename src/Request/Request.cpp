#include "Request.hpp"

Request::Request(const std::string &requestLine)
{
	// write a regex expression to parse http version
	std::regex httpVersionRegex("HTTP/\\d\\.\\d");
	std::regex requestLineRegex("HTTP/\\d\\.\\d");
}
