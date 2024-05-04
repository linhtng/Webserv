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
	std::string _target;
	ConnectionValue _connection;
	int _httpVersionMajor;
	int _httpVersionMinor;
	std::string _boundary;
	bool _criticalError;
	size_t _contentLength;
	bool _chunked;
	std::vector<std::byte> _body;
	std::chrono::system_clock::time_point _date;
	ContentType _contentType;
	std::unordered_map<std::string, std::string> _contentTypeParams;

	HttpMessage(ConfigData const &config,
				HttpStatusCode statusCode = HttpStatusCode::UNDEFINED_STATUS,
				HttpMethod method = HttpMethod::UNDEFINED_METHOD,
				std::string target = "",
				ConnectionValue connection = KEEP_ALIVE,
				int httpVersionMajor = 1,
				int httpVersionMinor = 1,
				std::string boundary = "",
				bool criticalError = false,
				size_t contentLength = 0,
				bool chunked = false);

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
	ContentType getContentType() const;
	HttpStatusCode getStatusCode() const;
	std::string getBoundary() const;
	bool getCriticalError() const;
	bool isChunked() const;
};

#endif
