#include "Request.hpp"

RequestStatus Request::getStatus() const
{
	return this->_status;
}

HttpStatusCode Request::getStatusCode() const
{
	return this->_statusCode;
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

void Request::appendToBody(const std::vector<std::byte> &newBodyChunk)
{
	this->_body.insert(this->_body.end(), newBodyChunk.begin(), newBodyChunk.end());
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
	std::regex requestLineRegex("^(GET|HEAD|POST)" SP "(.+)" SP "HTTP/(\\d{1,3})(\\.\\d{1,3})?$"); // nginx takes up to 3 digits for the minor version
	std::smatch match;
	if (std::regex_match(requestLine, match, requestLineRegex))
	{
		this->_requestLine.method = match[1];
		this->_requestLine.requestTarget = match[2];
		this->_requestLine.HTTPVersionMajor = match[3];
		/* if (match[4].matched)
		{
			this->_requestLine.HTTPVersionMinor = match[4].str().substr(1);
		}
		else
		{
			this->_requestLine.HTTPVersionMinor = "0";
		} */
		this->_status = RequestStatus::SUCCESS;
	}
	else
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		this->_status = RequestStatus::ERROR;
	}
}

void Request::validateRequestLine()
{
	// METHOD validation
	std::vector<std::string> validMethods = VALID_HTTP_METHODS;
	auto itValid = std::find(validMethods.begin(), validMethods.end(), this->_requestLine.method);
	if (itValid == validMethods.end())
	{
		// Error for invalid methods
		this->_statusCode = HttpStatusCode::METHOD_NOT_ALLOWED;
		this->_status = RequestStatus::ERROR;
		return;
	}
	// Success for implemented methods
	if (this->_requestLine.method == "GET")
	{
		this->_method = HttpMethod::GET;
	}
	else if (this->_requestLine.method == "HEAD")
	{
		this->_method = HttpMethod::HEAD;
	}
	else if (this->_requestLine.method == "POST")
	{
		this->_method = HttpMethod::POST;
	}
	else if (this->_requestLine.method == "DELETE")
	{
		this->_method = HttpMethod::DELETE;
	}
	else
	{
		// Error for valid but not implemented methods
		this->_statusCode = HttpStatusCode::NOT_IMPLEMENTED;
		this->_status = RequestStatus::ERROR;
		return;
	}

	// TARGET validation - is any needed?
	this->_requestTarget = this->_requestLine.requestTarget;

	// HTTP VERSION validation
	int major = std::stoi(this->_requestLine.HTTPVersionMajor);
	if (major > 1)
	{
		this->_statusCode = HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED;
		this->_status = RequestStatus::ERROR;
	}
	else if (major < 1)
	{
		this->_statusCode = HttpStatusCode::UPGRADE_REQUIRED;
		this->_status = RequestStatus::ERROR;
	}
}

void Request::parseHeaderLine(const std::string &headerLine)
{
	std::string chars = CR LF NUL;
	std::regex headerLineRegex("^([^" + chars + "]+):" SP "([^" + chars + "]+)$");
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
	: _statusCode(HttpStatusCode::UNDEFINED),
	  _status(RequestStatus::SUCCESS)
{
	if (requestLineAndHeaders.empty())
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		this->_status = RequestStatus::ERROR;
		return;
	}

	std::vector<std::string> split = splitByCRLF(requestLineAndHeaders);

	parseRequestLine(split[0]);
	if (this->_status == RequestStatus::ERROR)
	{
		return;
	}
	validateRequestLine();
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
	// validateHeaders();
}
