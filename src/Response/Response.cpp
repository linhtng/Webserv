#include "Response.hpp"

Response::Response(const Request &request)
{
	this->_body = request.getBody();
}

std::string Response::getHeader() const
{
	return "header!";
}

std::vector<std::byte> Response::getBody() const
{
	return this->_body;
}
