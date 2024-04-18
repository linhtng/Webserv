#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"

#include <string>
#include <chrono>

class Response : public HttpMessage
{
private:
	std::string _serverHeader;

public:
	Response(const Request &request);
	std::string getHeader() const;
};

#endif
