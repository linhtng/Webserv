#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "../defines.hpp"
#include <string>
#include <vector>
#include "../Server/Server.hpp"

class HttpMessage
{
protected:
	HttpMethod _method;
	std::string _target;
	int _httpVersionMajor;
	size_t _contentLength;
	std::vector<std::byte> _body;
	ConnectionValue _connection;
	std::chrono::system_clock::time_point _date;
	ContentType _contentType;

	HttpStatusCode _statusCode;
	bool _chunked;
	Server const &_server;

	HttpMessage(const HttpMessage &other) = delete;

	HttpMessage(Server const &server,
				HttpMethod method = HttpMethod::UNDEFINED,
				HttpStatusCode statusCode = HttpStatusCode::UNDEFINED,
				size_t contentLength = 0,
				bool chunked = false,
				ConnectionValue connection = KEEP_ALIVE,
				int httpVersionMajor = 1);

public:
	virtual ~HttpMessage() = default;

	Server const &getServer() const;
	HttpMethod getMethod() const;
	std::string getTarget() const;
	int getHttpVersionMajor() const;
	size_t getContentLength() const;
	std::vector<std::byte> getBody() const;
	ConnectionValue getConnection() const;
	std::chrono::system_clock::time_point getDate() const;

	HttpStatusCode getStatusCode() const;
	bool isChunked() const;
};

#endif
