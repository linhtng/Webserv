#include "Request.hpp"
#include <iostream>

void Request::printRequestProperties() const
{
	std::cout << "Method: " << this->_method << std::endl;
	std::cout << "Request target: " << this->_target << std::endl;
	std::cout << "HTTP version major: " << this->_httpVersionMajor << std::endl;
	std::cout << "Status code: " << this->_statusCode << std::endl;
	std::cout << "Host: " << this->_host << std::endl;
	std::cout << "Port: " << this->_port << std::endl;
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

size_t Request::getChunkSize() const
{
	return this->_chunkSize;
}

std::string Request::getBodyBuf() const
{
	return this->_bodyBuf;
}

// MODIFIERS

void Request::appendToBody(const std::vector<std::byte> &newBodyChunk)
{
	this->_body.insert(this->_body.end(), newBodyChunk.begin(), newBodyChunk.end());
}

void Request::setChunkSize(const size_t &bytes)
{
	_chunkSize = bytes;
}

void Request::setBodyBuf(const std::string &buf)
{
	_bodyBuf = buf;
}

// UTILITIES

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
	std::regex hostRegex(HOST_REGEX);
	std::smatch match;
	if (!std::regex_match(this->_headerLines["host"], match, hostRegex))
	{
		throw BadRequestException();
	}
	this->_host = match[1];
	this->_port = std::stoi(match[2]);
	// TODO: handle exceptionss
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
	this->_contentLength = StringUtils::strToSizeT(contentLengthValue);
	// overflow would be 400 and 413 should be thrown if value is bigger than max body size from config
	if (this->_contentLength > this->_config.getMaxClientBodySize())
	{
		this->_statusCode = HttpStatusCode::PAYLOAD_TOO_LARGE;
		throw BadRequestException();
	}
	if (this->_contentLength > 0)
	{
		this->_bodyExpected = true;
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
		this->_bodyExpected = false;
		throw BadRequestException();
	}
	std::string transferEncodingValue = it->second;
	std::transform(transferEncodingValue.begin(), transferEncodingValue.end(), transferEncodingValue.begin(), ::tolower);
	if (transferEncodingValue == "chunked")
	{
		this->_chunked = true;
		this->_bodyExpected = true;
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

void Request::parseContentType()
{
	auto it = this->_headerLines.find("content-type");
	if (it == this->_headerLines.end())
	{
		return;
	}
	std::string contentTypeFullValue = it->second;
	std::transform(contentTypeFullValue.begin(), contentTypeFullValue.end(), contentTypeFullValue.begin(), ::tolower);

	// split by semicolon
	std::vector<std::string> split = StringUtils::splitByDelimiter(contentTypeFullValue, ";");
	this->_contentType = StringUtils::trim(split[0]);
	// is there are subtype params, parse them
	for (size_t i = 1; i < split.size(); ++i)
	{
		std::vector<std::string> paramsSplit = StringUtils::splitByDelimiter(split[i], "=");
		if (paramsSplit.size() != 2)
		{
			throw BadRequestException();
		}
		this->_contentTypeParams[StringUtils::trim(paramsSplit[0])] = StringUtils::trim(paramsSplit[1]);
	}
	if (this->_contentType == "multipart/form-data")
	{
		// check for boundary
		auto it = this->_contentTypeParams.find("boundary");
		if (it == this->_contentTypeParams.end())
		{
			throw BadRequestException();
		}
		// check if boundary is valid
		std::string boundary = it->second;
		if (boundary.empty())
		{
			throw BadRequestException();
		}
		this->_boundary = boundary;
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
	parseContentType();
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
	std::vector<std::string> split = StringUtils::splitByDelimiter(requestLineAndHeaders, CRLF);
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

Request::Request(const ConfigData &config, const std::string &requestLineAndHeaders)
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

Request::Request(const ConfigData &config, HttpStatusCode statusCode)
	: HttpMessage(config, statusCode)
{
}