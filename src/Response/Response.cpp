#include "Response.hpp"

Response::Response(const Request &request) : HttpMessage(request)
{
}

std::string Response::getHeader() const
{
	return "header!";
}
