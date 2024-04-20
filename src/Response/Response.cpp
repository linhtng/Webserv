#include "Response.hpp"

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getMethod(), request.getStatusCode(), request.getContentLength(), request.isChunked(), request.getConnection(), request.getHttpVersionMajor()), _request(request)
{
	(void)_request;
}

std::string Response::getHeader() const
{
	return "header!";
}
