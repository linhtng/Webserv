#include "Response.hpp"

// DEBUGGING FUNCTIONS

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

// STRING FORMING FUNCTIONS

std::string Response::formatDate() const
{
	std::time_t currentTime = std::chrono::system_clock::to_time_t(this->_date);
	std::stringstream ss;
	ss << std::put_time(std::gmtime(&currentTime), "%a, %d %b %Y %H:%M:%S GMT");
	std::string httpDate = ss.str();
	return httpDate;
}

std::string Response::formatStatusCodeMessage() const
{
	return (std::to_string(this->_statusCode) + " " + this->_statusCodeMessages.at(this->_statusCode));
}

std::string Response::formatConnection() const
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

std::string Response::formatContentType() const
{
	return "text/html";
}

std::string Response::formatStatusLine() const
{
	std::string statusLine;
	statusLine += std::to_string(this->_httpVersionMajor) + "." + std::to_string(this->_httpVersionMinor) + " " + this->formatStatusCodeMessage();
	return statusLine;
}

std::string Response::formatHeader() const
{
	std::string header;
	header += this->formatStatusLine() + CRLF;
	header += "Date: " + this->formatDate() + CRLF;
	header += "Server: " + this->_config.getServerName() + CRLF;
	header += "Content-Length: " + std::to_string(this->_body.size()) + CRLF;
	header += "Content-Type: " + this->formatContentType() + CRLF;
	header += "Connection: " + this->formatConnection() + CRLF;
	header += CRLF;
	return header;
}

std::vector<std::byte> Response::formatResponse() const
{
	std::vector<std::byte> response;
	for (char ch : this->formatHeader())
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	std::vector<std::byte> body = this->_body;
	response.insert(response.end(), body.begin(), body.end());
	return response;
}

// RESPONSE PREPARATION

void Response::setDateToCurrent()
{
	this->_date = std::chrono::system_clock::now();
}

void Response::prepareResponse()
{
	// Things that are done for every response
	this->setDateToCurrent();
	this->_serverHeader = this->_config.getServerName();
	this->_connection = this->_request.getConnection();
}

void Response::prepareErrorResponse()
{
	this->_body = DefaultResponsePage::getResponsePage(this->_statusCode);
}

// CONSTRUCTOR

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod()), _request(request)
{
	if (this->_statusCode != HttpStatusCode::UNDEFINED_STATUS)
	{
		// we already know what the response will be, just need to format it
		this->prepareErrorResponse();
	}
	else
	{
		//
	}
}
