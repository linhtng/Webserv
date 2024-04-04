#include "Request.hpp"

RequestStatus Request::getStatus() const
{
	return this->_status;
}

bool Request::bodyExpected() const
{
	// also consider Content-Length == "0"
	if (this->_headerLines.find("Content-Length") != this->_headerLines.end() ||
		this->_headerLines.find("Transfer-Encoding") != this->_headerLines.end())
	{
		return true;
	}
	return false;
}

std::vector<std::byte> Request::getBody() const
{
	return this->_body;
}

void Request::setBody(const std::vector<std::byte> &body)
{
	this->_body = body;
}

size_t Request::getContentLength() const
{
	// TODO: handle exceptions, or better yet validate before calling
	if (this->_headerLines.find("Content-Length") != this->_headerLines.end())
	{
		return std::stoul(this->_headerLines.at("Content-Length"));
	}
	return 0;
}

std::vector<std::string> splitByCRLF(const std::string &input)
{
	std::vector<std::string> result;
	size_t pos = 0;
	size_t prev = 0;
	while ((pos = input.find(CRLF, prev)) != std::string::npos)
	{
		if (pos > prev)
			result.push_back(input.substr(prev, pos - prev));
		prev = pos + 2; // Move past the CRLF
	}
	// Add the last substring if it exists
	if (prev < input.length())
		result.push_back(input.substr(prev, std::string::npos));
	return result;
}

void Request::parseRequestLine(const std::string &requestLine)
{
	std::regex requestLineRegex("^(GET|HEAD|POST)" SP "(.+)" SP "HTTP/(\d+\\.\d+)$"); // nginx takes up to 3 digits for the minor version
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
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		this->_status = RequestStatus::ERROR;
	}
}

void Request::parseHeaderLine(const std::string &headerLine)
{
	std::regex headerLineRegex("^([^" CR LF NUL "]+):" SP "([^" CR LF NUL "]+)$");
	std::smatch match;
	if (std::regex_match(headerLine, match, headerLineRegex))
	{
		this->_headerLines[match[1]] = match[2];
	}
	else
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		this->_status = RequestStatus::ERROR;
	}
}

Request::Request(const std::string &requestLineAndHeaders)
	: _statusCode(HttpStatusCode::UNDEFINED)
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
