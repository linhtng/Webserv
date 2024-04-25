#include "Response.hpp"

void Response::setDateToCurrent()
{
	this->_date = std::chrono::system_clock::now();
}

std::string Response::formDate() const
{
	std::time_t currentTime = std::chrono::system_clock::to_time_t(this->_date);
	std::stringstream ss;
	ss << std::put_time(std::gmtime(&currentTime), "%a, %d %b %Y %H:%M:%S GMT");
	std::string httpDate = ss.str();
	return httpDate;
}

std::string Response::formStatusCodeMessage() const
{
	return this->_statusCodeMessages.at(this->_statusCode);
}

std::string Response::formConnection() const
{
	if (this->_connection == ConnectionValue::CLOSE)
	{
		return "close";
	}
	else
	{
		return "keep-alive";
	}
}

std::string Response::formContentType() const
{
	return "text/html";
}

std::string Response::formHeader() const
{
	std::string header;
	header += this->_httpVersionMajor + " " + this->formStatusCodeMessage() + CRLF;
	header += "Date: " + this->formDate() + CRLF;
	header += "Server: " + this->_config.getServerName() + CRLF;
	header += "Content-Length: " + std::to_string(this->_body.size()) + CRLF;
	header += "Content-Type: " + this->formContentType() + CRLF;
	header += "Connection: " + this->formConnection() + CRLF;
	header += CRLF;
	return header;
}

std::vector<std::byte> Response::formResponse() const
{
	std::vector<std::byte> response;
	for (char ch : this->formHeader())
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	std::vector<std::byte> body = this->_body;
	response.insert(response.end(), body.begin(), body.end());
	return response;
}

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
