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
	ConnectionValue _connection;
	size_t _contentLength;
	bool _chunked;
	int _httpVersionMajor;
	int _httpVersionMinor;
	std::string _target;
	std::vector<std::byte> _body;
	std::chrono::system_clock::time_point _date;
	std::string _contentType;
	std::unordered_map<std::string, std::string> _contentTypeParams;

	HttpMessage(ConfigData const &config,
				HttpStatusCode statusCode = HttpStatusCode::UNDEFINED_STATUS,
				HttpMethod method = HttpMethod::UNDEFINED_METHOD,
				std::string target = "",
				ConnectionValue connection = KEEP_ALIVE,
				size_t contentLength = 0,
				bool chunked = false,
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
