#include "HttpMessage.hpp"

const std::unordered_map<HttpStatusCode, std::string> HttpMessage::_statusCodeMessages = {
	{CONTINUE, "Continue"},
	{SWITCHING_PROTOCOLS, "Switching Protocols"},
	{PROCESSING, "Processing"},
	{OK, "OK"},
	{CREATED, "Created"},
	{ACCEPTED, "Accepted"},
	{NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
	{NO_CONTENT, "No Content"},
	{RESET_CONTENT, "Reset Content"},
	{PARTIAL_CONTENT, "Partial Content"},
	{MULTIPLE_CHOICES, "Multiple Choices"},
	{MOVED_PERMANENTLY, "Moved Permanently"},
	{FOUND, "Found"},
	{SEE_OTHER, "See Other"},
	{NOT_MODIFIED, "Not Modified"},
	{TEMPORARY_REDIRECT, "Temporary Redirect"},
	{PERMANENT_REDIRECT, "Permanent Redirect"},
	{BAD_REQUEST, "Bad Request"},
	{UNAUTHORIZED, "Unauthorized"},
	{FORBIDDEN, "Forbidden"},
	{NOT_FOUND, "Not Found"},
	{METHOD_NOT_ALLOWED, "Method Not Allowed"},
	{NOT_ACCEPTABLE, "Not Acceptable"},
	{PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
	{REQUEST_TIMEOUT, "Request Timeout"},
	{CONFLICT, "Conflict"},
	{GONE, "Gone"},
	{LENGTH_REQUIRED, "Length Required"},
	{PRECONDITION_FAILED, "Precondition Failed"},
	{PAYLOAD_TOO_LARGE, "Payload Too Large"},
	{URI_TOO_LONG, "URI Too Long"},
	{UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
	{RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
	{EXPECTATION_FAILED, "Expectation Failed"},
	{IM_A_TEAPOT, "I'm a teapot"},
	{UPGRADE_REQUIRED, "Upgrade Required"},
	{PRECONDITION_REQUIRED, "Precondition Required"},
	{TOO_MANY_REQUESTS, "Too Many Requests"},
	{REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
	{UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},
	{INTERNAL_SERVER_ERROR, "Internal Server Error"},
	{NOT_IMPLEMENTED, "Not Implemented"},
	{BAD_GATEWAY, "Bad Gateway"},
	{SERVICE_UNAVAILABLE, "Service Unavailable"},
	{GATEWAY_TIMEOUT, "Gateway Timeout"},
	{HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"}};

HttpMessage::HttpMessage(ConfigData const &config,
						 HttpStatusCode statusCode,
						 HttpMethod method,
						 size_t contentLength,
						 bool chunked,
						 ConnectionValue connection,
						 int httpVersionMajor,
						 int httpVersionMinor)
	: _config(config),
	  _statusCode(statusCode),
	  _method(method),
	  _contentLength(contentLength),
	  _chunked(chunked),
	  _connection(connection),
	  _httpVersionMajor(httpVersionMajor),
	  _httpVersionMinor(httpVersionMinor)
{
}

ConfigData const &HttpMessage::getConfig() const
{
	return this->_config;
}

std::vector<std::byte> HttpMessage::getBody() const
{
	return this->_body;
}

HttpStatusCode HttpMessage::getStatusCode() const
{
	return this->_statusCode;
}

size_t HttpMessage::getContentLength() const
{
	return this->_contentLength;
}

ConnectionValue HttpMessage::getConnection() const
{
	return this->_connection;
}

std::chrono::system_clock::time_point HttpMessage::getDate() const
{
	return this->_date;
}

HttpMethod HttpMessage::getMethod() const
{
	return this->_method;
}

std::string HttpMessage::getTarget() const
{
	return this->_target;
}

int HttpMessage::getHttpVersionMajor() const
{
	return this->_httpVersionMajor;
}

int HttpMessage::getHttpVersionMinor() const
{
	return this->_httpVersionMinor;
}

bool HttpMessage::isChunked() const
{
	return this->_chunked;
}

std::string HttpMessage::getContentType() const
{
	return this->_contentType;
}
