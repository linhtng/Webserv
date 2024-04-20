#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../Request/HttpMessage.hpp"

#include <string>
#include <chrono>

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	std::string _serverHeader;
	Request const &_request;

public:
	Response(const Request &request);
	std::string getHeader() const;
};

#endif
