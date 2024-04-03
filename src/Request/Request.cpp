#include "Request.hpp"

void Request::parseRequestLine(const std::string &requestLine)
{
	std::regex requestLineRegex("^(GET|POST)" SP "(.+)" SP "HTTP/(\d+\\.\d+)$");
	std::smatch match;
	if (std::regex_match(requestLine, match, requestLineRegex))
	{
		this->_requestLine.method = match[1];
		this->_requestLine.requestTarget = match[2];
		this->_requestLine.HTTPVersion = match[3];
		this->_status = RequestStatus::SUCCESS;
	}
	else
	{
		this->_status = RequestStatus::ERROR;
	}
}

void Request::parseHeaderLine(const std::string &headerLine)
{
	std::regex headerLineRegex("^(.+):" SP "(.+)$");
	std::smatch match;
	if (std::regex_match(headerLine, match, headerLineRegex))
	{
		this->_headerLines[match[1]] = match[2];
	}
	else
	{
		this->_status = RequestStatus::ERROR;
	}
}

Request::Request(const std::string &requestLineAndHeaders)
{
	std::vector<std::string> split = splitByCRLF(requestLineAndHeaders);

	parseRequestLine(split[0]);
	if (this->_status == RequestStatus::ERROR)
	{
		return;
	}
	for (size_t i = 1; i < split.size(); ++i)
	{
		parseHeaderLine(split[i]);
		if (this->_status == RequestStatus::ERROR)
		{
			return;
		}
	}
}
