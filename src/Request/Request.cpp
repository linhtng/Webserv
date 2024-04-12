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

void Request::appendToBody(const std::vector<std::byte> &newBodyChunk)
{
	this->_body.insert(this->_body.end(), newBodyChunk.begin(), newBodyChunk.end());
}

std::vector<std::string> splitByCRLF(const std::string &input)
{
	std::vector<std::string> result;
	size_t pos = 0;
	size_t prev = 0;
	while ((pos = input.find(CRLF, prev)) != std::string::npos)
	{
		if (pos > prev)
		{
			result.push_back(input.substr(prev, pos - prev));
		}
		prev = pos + 2; // Move past CRLF
	}
	// Add the last substring if it exists
	if (prev < input.length())
	{
		result.push_back(input.substr(prev, std::string::npos));
	}
	return result;
}

void Request::extractRequestLine(const std::string &requestLine)
{
	std::regex requestLineRegex(REQUEST_LINE_REGEX);
	std::smatch match;
	if (std::regex_match(requestLine, match, requestLineRegex))
	{
		this->_requestLine.method = match[1];
		this->_requestLine.requestTarget = match[2];
		this->_requestLine.HTTPVersionMajor = match[3];
		this->_status = RequestStatus::SUCCESS;
	}
	else
	{
		throw BadRequestException();
	}
}

void Request::validateMethod()
{
	std::vector<std::string> validMethods = VALID_HTTP_METHODS;
	auto itValid = std::find(validMethods.begin(), validMethods.end(), this->_requestLine.method);
	if (itValid == validMethods.end())
	{
		this->_statusCode = HttpStatusCode::METHOD_NOT_ALLOWED;
		throw BadRequestException();
	}
}

HttpMethod Request::matchValidMethod()
{
	// is method case-sensitive? TODO: check RFC
	std::unordered_map<std::string, HttpMethod> methodMap = {
		{"GET", HttpMethod::GET},
		{"HEAD", HttpMethod::HEAD},
		{"POST", HttpMethod::POST},
		{"DELETE", HttpMethod::DELETE}};
	auto it = methodMap.find(this->_requestLine.method);
	if (it == methodMap.end())
	{
		this->_statusCode = HttpStatusCode::NOT_IMPLEMENTED;
		throw BadRequestException();
	}
	return it->second;
}

HttpMethod Request::parseMethod()
{
	validateMethod();
	return matchValidMethod();
}

int Request::parseVersion()
{
	// no need to handle exceptions because we already know it's 1-3 digits thanks to regex
	int major = std::stoi(this->_requestLine.HTTPVersionMajor);
	if (major > 1)
	{
		this->_statusCode = HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED;
		throw BadRequestException();
	}
	else if (major < 1)
	{
		this->_statusCode = HttpStatusCode::UPGRADE_REQUIRED;
		throw BadRequestException();
	}
	return major;
}

void Request::parseRequestLine()
{
	// METHOD validation

	this->_method = parseMethod();

	// TARGET validation - is any needed?
	this->_target = this->_requestLine.requestTarget;

	// HTTP VERSION validation
	this->_httpVersionMajor = parseVersion();
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
		throw BadRequestException();
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

std::string Request::removeComments(const std::string &input) const
{
	std::string res;
	std::string currentSegment;
	int commentLevel = 0;

	for (char c : input)
	{
		if (c == '(')
		{
			commentLevel++;
			res += currentSegment;
			currentSegment.clear();
		}
		else if (c == ')')
		{
			if (commentLevel == 0)
			{
				throw BadRequestException();
			}
			commentLevel--;
		}
		else if (commentLevel == 0)
		{
			currentSegment += c;
		}
	}

	return res;
}

size_t Request::strToSizeT(const std::string &str) const
{
	if (str.empty() || !isDigitsOnly(str))
	{
		throw BadRequestException();
	}
	try
	{
		unsigned long long value = std::stoull(str);
		if (value > std::numeric_limits<size_t>::max())
		{
			throw BadRequestException();
		}
		return static_cast<size_t>(value);
	}
	// ull overflow
	catch (const std::exception &e)
	{
		throw BadRequestException();
	}
}

void Request::parseContentLength()
{
	std::unordered_map<std::string, std::string>::iterator it = this->_headerLines.find("content-length");
	if (it == this->_headerLines.end())
	{
		return;
	}
	std::string contentLengthValue = it->second;
	/*
	Since there is no predefined limit to the length of content, a recipient MUST anticipate potentially large decimal numerals and prevent parsing errors due to integer conversion overflows or precision loss due to integer conversion
	*/
	this->_contentLength = strToSizeT(contentLengthValue);
	// overflow would be 400 and 413 should be thrown if value is bigger than max body size from config
	if (this->_contentLength > MAX_BODY_SIZE)
	{
		this->_statusCode = HttpStatusCode::CONTENT_TOO_LARGE;
		throw BadRequestException();
	}
}

void Request::parseUserAgent()
{
	std::unordered_map<std::string, std::string>::iterator it = this->_headerLines.find("user-agent");
	if (it == this->_headerLines.end())
	{
		return;
	}

	// TODO: regex stuff
}

void Request::parseHeaders()
{
	parseHost();
	parseContentLength();
	parseTransferEncoding();
	parseUserAgent();
	parseConnection();
}

void Request::processRequest(const std::string &requestLineAndHeaders)
{
	if (requestLineAndHeaders.empty())
	{
		throw BadRequestException();
	}
	std::vector<std::string> split = splitByCRLF(requestLineAndHeaders);
	// maybe rather than keeping RequestLine _requestLine in a class, just make it local to here
	extractRequestLine(split[0]);
	parseRequestLine();
	for (size_t i = 1; i < split.size(); ++i)
	{
		extractHeaderLine(split[i]);
	}
	parseHeaders();
}

Request::Request(const std::string &requestLineAndHeaders)
	: HttpMessage(HttpMethod::UNDEFINED, 0, 0, HttpStatusCode::UNDEFINED, false),
	  _status(RequestStatus::SUCCESS),
	  _bodyExpected(false)
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
