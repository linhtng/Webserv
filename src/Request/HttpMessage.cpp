#include "HttpMessage.hpp"

HttpMessage::HttpMessage(Server const &server,
						 HttpMethod method,
						 HttpStatusCode statusCode,
						 size_t contentLength,
						 bool chunked,
						 ConnectionValue connection,
						 int httpVersionMajor)
	: _method(method),
	  _httpVersionMajor(httpVersionMajor),
	  _contentLength(contentLength),
	  _statusCode(statusCode),
	  _chunked(chunked),
	  _connection(connection),
	  _server(server) {}

Server const &HttpMessage::getServer() const
{
	return this->_server;
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
