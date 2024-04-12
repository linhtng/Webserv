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

	HttpStatusCode _statusCode;
	bool _chunked;

	HttpMessage(HttpMethod method, int httpVersionMajor, size_t contentLength, HttpStatusCode statusCode, bool chunked) : _method(method), _httpVersionMajor(httpVersionMajor), _contentLength(contentLength), _statusCode(statusCode), _chunked(chunked){};

public:
	std::vector<std::byte> getBody() const;
	HttpStatusCode getStatusCode() const;
	size_t getContentLength() const;
};

#endif
