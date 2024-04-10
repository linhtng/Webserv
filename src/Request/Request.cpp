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

void Request::extractRequestLine(const std::string &requestLine)
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

void Request::parseRequestLine()
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

void Request::extractHeaderLine(const std::string &headerLine)
{
	std::string forbiddenChars = CR LF NUL;
	std::regex headerLineRegex("^([^" + forbiddenChars + "]+):" SP "([^" + forbiddenChars + "]+)$");
	std::smatch match;
	if (std::regex_match(headerLine, match, headerLineRegex))
	{
		std::string fieldName = match[1];
		std::transform(fieldName.begin(), fieldName.end(), fieldName.begin(), ::tolower);
		this->_headerLines[fieldName] = match[2];
	}
	else
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		this->_status = RequestStatus::ERROR;
	}
}

void Request::parseHost()
{
	if (this->_headerLines.find("host") == this->_headerLines.end())
	{
		throw BadRequestException();
	}
}

bool Request::isDigitsOnly(const std::string &str) const
{
	return std::all_of(
		str.begin(), str.end(), [](unsigned char c)
		{ return std::isdigit(c); });
}

void Request::parseContentLength()
{
	std::unordered_map<std::string, std::string>::iterator it = this->_headerLines.find("content-length");
	if (it == this->_headerLines.end())
	{
		return;
	}
	std::string contentLengthValue = it->second;
	if (!isDigitsOnly(contentLengthValue))
	{
		throw BadRequestException();
	}
	/*
	Since there is no predefined limit to the length of content, a recipient MUST anticipate potentially large decimal numerals and prevent parsing errors due to integer conversion overflows or precision loss due to integer conversion
	*/
	try
	{
		this->_contentLength = std::stoul(contentLengthValue);
	}
	catch (const std::exception &e)
	{
		// Maybe make these 2 lines a separate function?
		this->_statusCode = HttpStatusCode::CONTENT_TOO_LARGE;
		throw BadRequestException();
	}
	// TODO: actually, overflow would be 400 and 413 should be thrown if value is bigger than configured max body size
}

void Request::parseHeaders()
{
	parseHost();
	parseContentLength();
	/*
	parseContentType();
	parseAccept();
	parseUserAgent();
	parseConnection();
	parseAcceptEncoding();
	parseAcceptLanguage();
	*/
}

void Request::processRequest(const std::string &requestLineAndHeaders)
{
	if (requestLineAndHeaders.empty())
	{
		throw BadRequestException();
	}
	std::vector<std::string> split = splitByCRLF(requestLineAndHeaders);
	extractRequestLine(split[0]);
	parseRequestLine();
	for (size_t i = 1; i < split.size(); ++i)
	{
		extractHeaderLine(split[i]);
	}
	parseHeaders();
}

Request::Request(const std::string &requestLineAndHeaders)
	: _statusCode(HttpStatusCode::UNDEFINED),
	  _status(RequestStatus::SUCCESS)
{
	try
	{
		processRequest(requestLineAndHeaders);
	}
	catch (const BadRequestException &e)
	{
		if (this->_statusCode == UNDEFINED)
		{
			this->_statusCode = HttpStatusCode::BAD_REQUEST;
		}
		this->_status = RequestStatus::ERROR;
	}
	catch (const std::exception &e)
	{
		this->_statusCode = HttpStatusCode::INTERNAL_SERVER_ERROR;
		this->_status = RequestStatus::ERROR;
	}
}
