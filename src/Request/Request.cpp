#include "Request.hpp"
#include <iostream>

void Request::printRequestProperties() const
{
	std::cout << "Method: " << this->_method << std::endl;
	std::cout << "Request target: " << this->_target << std::endl;
	std::cout << "HTTP version major: " << this->_httpVersionMajor << std::endl;
	std::cout << "Status code: " << this->_statusCode << std::endl;
	std::cout << "Host: " << this->_host << std::endl;
	std::cout << "Content-Length: " << this->_contentLength << std::endl;
	std::cout << "Transfer-Encoding: " << this->_transferEncoding << std::endl;
	std::cout << "User-Agent: " << this->_userAgent << std::endl;
	std::cout << "Connection: " << this->_connection << std::endl;
}

// GETTERS

bool Request::isBodyExpected() const
{
	return this->_bodyExpected;
}

std::string Request::getUserAgent() const
{
	return this->_userAgent;
}

std::string Request::getHost() const
{
	return this->_host;
}

std::string Request::getTransferEncoding() const
{
	return this->_transferEncoding;
}

// MODIFIERS

void Request::appendToBody(const std::vector<std::byte> &newBodyChunk)
{
	this->_body.insert(this->_body.end(), newBodyChunk.begin(), newBodyChunk.end());
}

// UTILITIES

std::vector<std::string> Request::splitByDelimiter(const std::string &input, const std::string &delimiter) const
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

std::vector<std::string> Request::splitByCRLF(const std::string &input) const
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

std::vector<std::string> Request::splitCommaSeparatedList(const std::string &input) const
{
	std::vector<std::string> result;
	size_t pos = 0;
	size_t prev = 0;
	while ((pos = input.find(",", prev)) != std::string::npos)
	{
		if (pos > prev)
		{
			result.push_back(input.substr(prev, pos - prev));
		}
		prev = pos + 1; // Move past comma
	}
	// Add the last substring if it exists
	if (prev < input.length())
	{
		result.push_back(input.substr(prev, std::string::npos));
	}
	return result;
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

// PARSING

// REQUEST LINE

void Request::extractRequestLine(const std::string &requestLine)
{
	std::regex requestLineRegex(REQUEST_LINE_REGEX);
	std::smatch match;
	if (std::regex_match(requestLine, match, requestLineRegex))
	{
		this->_requestLine.method = match[1];
		this->_requestLine.requestTarget = match[2];
		this->_requestLine.HTTPVersionMajor = match[3];
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

		/*
		ERROR PAGE EXAMPLE:

		HTTP/1.1 426 Upgrade Required
		Upgrade: HTTP/2.0
		Connection: Upgrade
		Content-Length: 53
		Content-Type: text/plain

		This service requires use of the HTTP/2.0 protocol
		*/
	}
	return major;
}

void Request::parseRequestLine()
{
	this->_method = parseMethod();
	this->_target = this->_requestLine.requestTarget;
	this->_httpVersionMajor = parseVersion();
}

// HEADERS

void Request::parseHost()
{
	if (this->_headerLines.find("host") == this->_headerLines.end())
	{
		throw BadRequestException();
	}
}

void Request::parseContentLength()
{
	auto it = this->_headerLines.find("content-length");
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

void Request::parseTransferEncoding()
{

	/*
	chunking an already chunked message is not allowed
	If any transfer coding other than chunked is applied to a request's content, the sender MUST apply chunked as the final transfer coding to ensure that the message is properly framed.
	If any transfer coding other than chunked is applied to a response's content, the sender MUST either apply chunked as the final transfer coding or terminate the message by closing the connection.
	 */

	auto it = this->_headerLines.find("transfer-encoding");
	if (it == this->_headerLines.end())
	{
		return;
	}
	// protection from request smuggling
	if (this->_headerLines.find("content-length") != this->_headerLines.end())
	{
		throw BadRequestException();
	}
	std::string transferEncodingValue = it->second;
	std::transform(transferEncodingValue.begin(), transferEncodingValue.end(), transferEncodingValue.begin(), ::tolower);
	if (transferEncodingValue == "chunked")
	{
		this->_chunked = true;
	}
	else
	{
		// should I check for valid values and throw 400 if there's some invalid bs? or is this enough
		this->_statusCode = HttpStatusCode::NOT_IMPLEMENTED;
		throw BadRequestException();
	}
}

void Request::parseUserAgent()
{
	auto it = this->_headerLines.find("user-agent");
	if (it == this->_headerLines.end())
	{
		return;
	}
	// std::regex userAgentRegex(ALLOWED_CHARS_REGEX);
	std::regex userAgentRegex(R"(^[^\x00-\x1F\x7F()<>@,;:\\\\"/[\]?={} \x20\x7E]*$)");
	if (!std::regex_match(it->second, userAgentRegex))
	{
		std::cout << "bad user-agent: " << it->second << std::endl;
		throw BadRequestException();
	}
	this->_userAgent = it->second;
}

void Request::parseConnection()
{
	auto it = this->_headerLines.find("connection");
	if (it == this->_headerLines.end())
	{
		return;
	}
	std::string connectionValue = it->second;
	std::transform(connectionValue.begin(), connectionValue.end(), connectionValue.begin(), ::tolower);
	if (connectionValue == "close")
	{
		this->_connection = ConnectionValue::CLOSE;
	}
	else if (connectionValue != "keep-alive")
	{
		throw BadRequestException();
	}
}

// HEADERS GENERAL

void Request::parseHeaders()
{
	parseHost();
	parseContentLength();
	parseTransferEncoding();
	parseUserAgent();
	parseConnection();
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
		// std::cout << "fieldName: " << fieldName << ", value: " << match[2] << std::endl;
		this->_headerLines[fieldName] = match[2];
	}
	else
	{
		throw BadRequestException();
	}
}

// GENERAL

void Request::processRequest(const std::string &requestLineAndHeaders)
{
	if (requestLineAndHeaders.empty())
	{
		throw BadRequestException();
	}
	std::vector<std::string> split = splitByCRLF(requestLineAndHeaders);
	// handle request line
	extractRequestLine(split[0]);
	parseRequestLine();
	// handle headers
	for (size_t i = 1; i < split.size(); ++i)
	{
		extractHeaderLine(split[i]);
	}
	parseHeaders();
}

Request::Request(const std::string &requestLineAndHeaders, const ConfigData &config)
	: HttpMessage(config),
	  _bodyExpected(false)
{
	try
	{
		processRequest(requestLineAndHeaders);
		if (this->_statusCode == HttpStatusCode::UNDEFINED_STATUS)
		{
			this->_statusCode = HttpStatusCode::OK;
		}
	}
	catch (const BadRequestException &e)
	{
		if (this->_statusCode == HttpStatusCode::UNDEFINED_STATUS)
		{
			this->_statusCode = HttpStatusCode::BAD_REQUEST;
		}
	}
	catch (const std::exception &e)
	{
		this->_statusCode = HttpStatusCode::INTERNAL_SERVER_ERROR;
	}
}

Request::Request(HttpStatusCode statusCode, const ConfigData &config)
	: HttpMessage(config),
	  _bodyExpected(false)
{
	this->_statusCode = statusCode;
}
