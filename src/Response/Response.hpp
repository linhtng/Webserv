#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../HttpMessage/HttpMessage.hpp"

#include <string>
#include <chrono>
#include <vector>
#include <iomanip>

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	std::string _serverHeader;
	Request const &_request;
	void setDateToCurrent();
	std::string formDate() const;
	std::string getHeader() const;
	std::string formStatusLine() const;
	std::string formHeader() const;
	std::string formStatusCodeMessage() const;
	std::string formConnection() const;
	std::string formContentType() const;

public:
	Response(const Request &request);

	std::vector<std::byte> formResponse() const;
};

#endif
