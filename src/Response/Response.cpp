#include "Response.hpp"

void printResponseProperties(const Response &response)
{
	std::cout << "Response properties:" << std::endl;
	std::cout << "ConfigData: " << response.getConfig().getServerName() << std::endl;
	std::cout << "Method: " << response.getMethod() << std::endl;
	std::cout << "Target: " << response.getTarget() << std::endl;
	std::cout << "HTTP version: " << response.getHttpVersionMajor() << "." << response.getHttpVersionMinor() << std::endl;
	std::cout << "Content length: " << response.getContentLength() << std::endl;
	std::cout << "Body: ";
	for (std::byte byte : response.getBody())
	{
		std::cout << static_cast<char>(byte);
	}
	std::cout << std::endl;
	std::cout << "Connection: " << response.getConnection() << std::endl;
	std::cout << "Date: " << response.getDate().time_since_epoch().count() << std::endl;
	std::cout << "Content type: " << response.getContentType() << std::endl;
	std::cout << "Status code: " << response.getStatusCode() << std::endl;
	std::cout << "Chunked: " << response.isChunked() << std::endl;
}

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

std::string Response::formStatusLine() const
{
	std::string statusLine;
	statusLine += std::to_string(this->_httpVersionMajor) + "." + std::to_string(this->_httpVersionMinor) + " " + this->formStatusCodeMessage();
	return statusLine;
}

std::string Response::formHeader() const
{
	std::string header;
	header += this->formStatusLine() + CRLF;
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
