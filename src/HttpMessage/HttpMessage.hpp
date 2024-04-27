#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "../defines.hpp"
#include <string>
#include <vector>
#include <chrono>
#include "../config_parser/ConfigData.hpp"
#include <unordered_map>

class HttpMessage
{
protected:
	ConfigData const &_config;

	HttpStatusCode _statusCode;
	HttpMethod _method;
	size_t _contentLength;
	bool _chunked;
	ConnectionValue _connection;
	int _httpVersionMajor;
	int _httpVersionMinor;
	std::string _target;
	std::vector<std::byte> _body;
	std::chrono::system_clock::time_point _date;
	std::string _contentType;
	std::unordered_map<std::string, std::string> _contentTypeParams;

	HttpMessage(const HttpMessage &other) = delete;

	HttpMessage(ConfigData const &_config,
				HttpStatusCode statusCode = HttpStatusCode::UNDEFINED_STATUS,
				HttpMethod method = HttpMethod::UNDEFINED_METHOD,
				size_t contentLength = 0,
				bool chunked = false,
				ConnectionValue connection = KEEP_ALIVE,
				int httpVersionMajor = 1,
				int httpVersionMinor = 1);

public:
	virtual ~HttpMessage() = default;

	ConfigData const &getConfig() const;
	HttpMethod getMethod() const;
	std::string getTarget() const;
	int getHttpVersionMajor() const;
	int getHttpVersionMinor() const;
	size_t getContentLength() const;
	std::vector<std::byte> getBody() const;
	ConnectionValue getConnection() const;
	std::chrono::system_clock::time_point getDate() const;
	std::string getContentType() const;
	HttpStatusCode getStatusCode() const;
	bool isChunked() const;

	static const std::unordered_map<HttpStatusCode, std::string> _statusCodeMessages;
};

#endif
