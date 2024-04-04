#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"

class Response
{
private:
	Response(){};
	std::vector<std::byte> _body;

public:
	Response(const Request &request);
	~Response(){};
	std::string getHeader() const;
	std::vector<std::byte> getBody() const;
};

#endif
