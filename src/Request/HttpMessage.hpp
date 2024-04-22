#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "../defines.hpp"
#include <string>
#include <vector>
#include "../config_parser/ConfigData.hpp"

class HttpMessage
{
protected:
	ConfigData const &_config;

	HttpMethod _method;
	HttpStatusCode _statusCode;
	size_t _contentLength;
	bool _chunked;
	ConnectionValue _connection;
	int _httpVersionMajor;
	std::string _target;
	std::vector<std::byte> _body;
	std::chrono::system_clock::time_point _date;
	ContentType _contentType;

	HttpMessage(const HttpMessage &other) = delete;

	HttpMessage(ConfigData const &_config,
				HttpMethod method = HttpMethod::UNDEFINED_METHOD,
				HttpStatusCode statusCode = HttpStatusCode::UNDEFINED_STATUS,
				size_t contentLength = 0,
				bool chunked = false,
				ConnectionValue connection = KEEP_ALIVE,
				int httpVersionMajor = 1);

public:
	virtual ~HttpMessage() = default;

	ConfigData const &getConfig() const;
	HttpMethod getMethod() const;
	std::string getTarget() const;
	int getHttpVersionMajor() const;
	size_t getContentLength() const;
	std::vector<std::byte> getBody() const;
	ConnectionValue getConnection() const;
	std::chrono::system_clock::time_point getDate() const;
	ContentType getContentType() const;
	HttpStatusCode getStatusCode() const;
	bool isChunked() const;
};

#endif
