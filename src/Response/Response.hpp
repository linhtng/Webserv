#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../HttpMessage/HttpMessage.hpp"
#include "../DefaultErrorPage/DefaultErrorPage.hpp"

#include <string>
#include <chrono>
#include <vector>

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	std::string _serverHeader;
	std::chrono::system_clock::time_point _lastModified;

	Request const &_request;

	std::string formatDate() const;
	std::string getHeader() const;
	std::string formatStatusLine() const;
	std::string formatHeader() const;
	std::string formatStatusCodeMessage() const;
	std::string formatConnection() const;
	std::string formatContentType() const;

	void setDateToCurrent();
	void prepareResponse();
	void prepareErrorResponse();
	void prepareStandardHeaders();

public:
	Response(const Request &request);

	std::vector<std::byte> formatResponse() const;
};

#endif
