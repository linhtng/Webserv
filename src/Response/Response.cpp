#include "Response.hpp"

// DEBUGGING FUNCTIONS

void printResponseProperties(const Response &response)
{
	std::cout << "Response properties:" << std::endl;
	std::cout << "Server name: " << response.getConfig().getServerName() << std::endl;
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
	// TODO: actually handle different types
	return "text/html";
}

std::string Response::formatStatusLine() const
{
	std::string statusLine;
	statusLine += "HTTP/" + std::to_string(this->_httpVersionMajor) + "." + std::to_string(this->_httpVersionMinor) + " " + this->formatStatusCodeMessage();
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

/* std::string getLastModified(const std::filesystem::path &filePath)
{
	try
	{
		auto lastModifiedTime = std::filesystem::last_write_time(filePath);
		// Convert file_time_type to time_t
		std::time_t lastModifiedTimeT = std::filesystem::file_time_type::clock::to_time_t(lastModifiedTime);
		std::tm *time = std::localtime(&lastModifiedTimeT);
		if (time)
		{
			char buffer[100];
			std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", time);
			return std::string(buffer);
		}
	}
	catch (const std::filesystem::filesystem_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return "Unknown";
} */
/*
std::string setLastModified(const std::filesystem::path &filePath)
{
	try
	{
		auto lastModifiedTime = std::filesystem::last_write_time(filePath);
		// Convert file_time_type to time_t
		std::time_t lastModifiedTimeT = std::filesystem::file_time_type::clock::to_time_t(lastModifiedTime);
		std::tm *time = std::localtime(&lastModifiedTimeT);
	}
	catch (const std::filesystem::filesystem_error &e)
	{
	}
} */

void Response::setDateToCurrent()
{
	this->_date = std::chrono::system_clock::now();
}

void Response::prepareResponse()
{
	// Things that are done for every response
}

void Response::prepareErrorResponse()
{
	this->_body = DefaultErrorPage::getErrorPage(this->_statusCode);
	this->_contentLength = this->_body.size();
	// set Content-Type to http
}

void Response::prepareStandardHeaders()
{
	this->_httpVersionMajor = 1;
	this->_httpVersionMinor = 1;
	this->setDateToCurrent();
	this->_serverHeader = this->_config.getServerName();
	this->_connection = this->_request.getConnection();
}

// CONSTRUCTOR

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod()), _request(request)
{
	this->prepareStandardHeaders();
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
