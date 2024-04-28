#include "Response.hpp"

// DEBUGGING FUNCTIONS

void Response::printResponseProperties() const
{
	std::cout << "Response properties:" << std::endl;
	std::cout << "Server name: " << this->getConfig().getServerName() << std::endl;
	std::cout << "Method: " << this->getMethod() << std::endl;
	std::cout << "Target: " << this->getTarget() << std::endl;
	std::cout << "HTTP version: " << this->getHttpVersionMajor() << "." << this->getHttpVersionMinor() << std::endl;
	std::cout << "Content length: " << this->getContentLength() << std::endl;
	std::cout << "Connection: " << this->getConnection() << std::endl;
	std::cout << "Date: " << this->getDate().time_since_epoch().count() << std::endl;
	std::cout << "Content type: " << this->getContentType() << std::endl;
	std::cout << "Status code: " << this->getStatusCode() << std::endl;
	std::cout << "Chunked: " << this->isChunked() << std::endl;
	std::cout << "Body: ";
	for (std::byte byte : this->getBody())
	{
		std::cout << static_cast<char>(byte);
	}
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
	printResponseProperties();
	std::string header;
	header += this->formatStatusLine() + CRLF;
	header += "Date: " + this->formatDate() + CRLF;
	header += "Server: " + this->_config.getServerName() + CRLF;
	header += "Content-Length: XXX" CRLF;
	header += "Content-Length: " + std::to_string(this->_body.size()) + CRLF;
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

void Response::prepareRedirectResponse()
{
	this->_statusCode = HttpStatusCode::MOVED_PERMANENTLY;
	this->_locationHeader = this->_redirectionRoute;
}

// CONSTRUCTOR

bool Response::isRedirect()
{
	this->_redirectionRoute = this->_location.getRedirectionRoute();
	return this->_redirectionRoute != "";
}

bool Response::targetFound()
{
	if (!FileSystemUtils::pathExists(this->_target))
	{
		this->_statusCode = HttpStatusCode::NOT_FOUND;
	}
	else
	{
		// check permissions
	}
}

bool Response::isCGI()
{
	return false;
}

void Response::executeCGI()
{
}

void Response::handlePost()
{
}

void Response::handleGet()
{
	if (FileSystemUtils::isDir(this->_target))
	{
		if (this->_location.getDirectoryListing())
		{
			// serve directory listing
			this->_body = DirectoryListingPage::getDirectoryListingPage(this->_target);
			this->_statusCode = HttpStatusCode::OK;
		}
		else if (!this->_location.getDefaultFile().empty())
		{
			// server index file
		}
		else
		{
			// 403 Forbidden
		}
	}
	else
	{
		if (FileSystemUtils::isFile(this->_target))
		{
			// serve file
		}
		else
		{
			// 404 Not Found
		}
	}
}

void Response::handleHead()
{
}

void Response::handleDelete()
{
}

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod(), request.getTarget(), request.getConnection()), _request(request)
{
	this->prepareStandardHeaders();
	if (this->_statusCode != HttpStatusCode::UNDEFINED_STATUS)
	{
		// we already know what the response will be, just need to format it
		this->prepareErrorResponse();
	}
	else
	{
		// try to match location
		if (_config.hasMatchingLocation(_target))
		{
			this->_location = _config.getMatchingLocation(_target);
		}
		else
		{
			this->_statusCode = HttpStatusCode::NOT_FOUND;
			this->prepareErrorResponse();
			return;
		}
		// handle redirection
		if (isRedirect())
		{
			this->prepareRedirectResponse();
			return;
		}
		// make sure target exists
		if (!targetFound())
		{
			this->_statusCode = HttpStatusCode::NOT_FOUND;
			this->prepareErrorResponse();
			return;
		}
		if (isCGI())
		{
			executeCGI();
		}
		else
		{
			switch (this->_method)
			{
			case HttpMethod::POST:
				handlePost();
				break;
			case HttpMethod::GET:
				handleGet();
				break;
			case HttpMethod::HEAD:
				handleHead();
				break;
			case HttpMethod::DELETE:
				handleDelete();
				break;
			default:
				// 405 Method Not Allowed
				// this was already checked in the Request class though
				this->_statusCode = HttpStatusCode::METHOD_NOT_ALLOWED;
				this->prepareErrorResponse();
				break;
			}
		}

		// check target, if REDIRECT, return response with 301, 308 with Location header
		// check target, return 404 if resourse NOT FOUND
		// - CGI:
		//		- check permissions, reject if not allowed
		//		- execute the script
		//		- serve the output back to the client
		// check method, handle everey method separately
		// - POST:
		//		- is upload allowed? if not reject with 403
		//		- parse body and save it as a file
		//		- serve the file back to the client
		// - GET:
		//		- FILE:
		//			- check permissions, reject if not allowed with 403
		//			- serve the file back to the client
		//		- DIRECTORY:
		//			- check permissions, reject if not allowed with 403
		//			- check if index exists, serve it if it does
		//			- otherwise check if autoindex is on, serve the directory listing back to the client or reject with 403
		// - HEAD:
		//		- check permissions, reject if not allowed with 403
		//		- serve the file back to the client, but without the body
		// - DELETE:
		//		- check permissions, reject if not allowed with 403
		//		- check if directory, reject if it is
		//		- delete the file
		//		- return headers with no body

		// Locations that are allowed - is it in config? ask Linh
	}
}

// RESPONSES:

// 200 OK
// 201 Created maybe for file upload?
// 204 No Content
// 301, 308 - redirection
// 403 Forbidden
// 404 Not Found

// function that forms the response based on status code