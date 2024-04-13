#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "../defines.hpp"
#include <string>
#include <vector>

class HttpMessage
{
protected:
	HttpMethod _method;
	std::string _target;
	int _httpVersionMajor;
	size_t _contentLength;
	std::vector<std::byte> _body;
	ConnectionValue _connection;

	HttpStatusCode _statusCode;
	bool _chunked;

	HttpMessage()
		: _method(HttpMethod::UNDEFINED),
		  _httpVersionMajor(1),
		  _contentLength(0),
		  _statusCode(HttpStatusCode::UNDEFINED),
		  _chunked(false),
		  _connection(KEEP_ALIVE){};

public:
	std::vector<std::byte> getBody() const;
	HttpStatusCode getStatusCode() const;
	ConnectionValue getConnection() const;
	size_t getContentLength() const;
};

#endif
