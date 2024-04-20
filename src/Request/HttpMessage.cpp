#include "HttpMessage.hpp"

HttpMessage::HttpMessage(ConfigData const &config,
						 HttpMethod method,
						 HttpStatusCode statusCode,
						 size_t contentLength,
						 bool chunked,
						 ConnectionValue connection,
						 int httpVersionMajor)
	: _config(config),
	  _method(method),
	  _statusCode(statusCode),
	  _contentLength(contentLength),
	  _chunked(chunked),
	  _connection(connection),
	  _httpVersionMajor(httpVersionMajor)
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

bool HttpMessage::isChunked() const
{
	return this->_chunked;
}
