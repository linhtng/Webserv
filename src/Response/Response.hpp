#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../HttpMessage/HttpMessage.hpp"
#include "../DefaultResponsePage/DefaultResponsePage.hpp"

#include <string>
#include <chrono>
#include <vector>

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	std::string _serverHeader;

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

public:
	Response(const Request &request);

	std::vector<std::byte> formatResponse() const;
};

#endif
