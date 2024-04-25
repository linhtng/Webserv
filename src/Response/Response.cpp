#include "Response.hpp"

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod()), _request(request)
{
	if (this->_statusCode >= HttpStatusCode::MULTIPLE_CHOICES)
	{
		// we already know what the response will be, just need to form it
		// formResponse();
	}
	else
	{
		//
	}
}

std::string Response::getHeader() const
{
	return "header!";
}
