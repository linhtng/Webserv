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
	// printResponseProperties();
	std::string header;
	header += this->formatStatusLine() + CRLF;
	header += "Date: " + this->formatDate() + CRLF;
	header += "Server: " + this->_config.getServerName() + CRLF;
	header += "Content-Length: " + std::to_string(this->_body.size()) + CRLF;
	header += "Connection: " + this->formatConnection() + CRLF;
	if (!this->_locationHeader.empty())
	{
		header += "Location: " + this->_locationHeader + CRLF;
	}
	header += CRLF;
	return header;
}

std::vector<std::byte> Response::formatResponse() const
{
	std::vector<std::byte> response;
	std::cout << RED << "Formatting response" << RESET << std::endl;
	for (char ch : this->formatHeader())
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	std::vector<std::byte> body = this->_body;
	std::cout << RED << "Formatted response" << RESET << std::endl;
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
}

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
}
 */
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
	this->_body = BinaryData::getErrorPage(this->_statusCode);
	this->_contentLength = this->_body.size();
	// set Content-Type to http
}

void Response::prepareStandardHeaders()
{
	/*
	this->_httpVersionMajor = 1;
	this->_httpVersionMinor = 1;
	*/
	this->setDateToCurrent();
	this->_serverHeader = SERVER_SOFTWARE;
	// lastModified should be set when we know the resource
}

void Response::prepareRedirectResponse()
{
	if (this->_method == HttpMethod::POST)
	{
		this->_statusCode = HttpStatusCode::PERMANENT_REDIRECT;
	}
	else
	{
		this->_statusCode = HttpStatusCode::MOVED_PERMANENTLY;
	}
	this->_locationHeader = this->_redirectionRoute;
}

// CONSTRUCTOR

bool Response::isRedirect()
{
	this->_redirectionRoute = this->_location.getRedirectionRoute();
	std::cout << RED << "Redirection route: " << this->_redirectionRoute << RESET << std::endl;
	return this->_redirectionRoute != "";
}

bool Response::targetFound()
{
	std::string fullPathNotTrimmed = StringUtils::joinPath(this->_location.getLocationRoot(), this->_route, this->_fileName);
	std::string fullPath = StringUtils::trimChar(fullPathNotTrimmed, '/');
	std::cout << GREEN << "Checking if path exists: " << fullPath << RESET << std::endl;
	if (!FileSystemUtils::pathExists(fullPath))
	{
		this->_statusCode = HttpStatusCode::NOT_FOUND;
		return false;
	}
	else
	{
		return true;
		// check permissions
	}
}

// probably would be nice to move this to utils
bool Response::extractFileName(const std::string &fileName, std::string &name, std::string &extension)
{
	std::vector<std::string> fileNameParts = StringUtils::splitByDelimiter(fileName, ".");
	if (fileNameParts.size() < 2)
	{
		return false;
	}
	extension = fileNameParts.back();
	name = fileName;
	return true;
}

// probably would be nice to move this to utils
void Response::splitTarget()
{
	// split string in two by last slash
	std::regex splittingPattern("^([a-zA-Z0-9/]*)/([^/]*)$");
	std::smatch matches;
	if (std::regex_match(this->_target, matches, splittingPattern))
	{
		std::string afterLastSlash = matches[matches.size() - 1];
		// is there was nothing after last slash, everything is route
		if (afterLastSlash.empty())
		{
			this->_route = _target;
		}
		else
		{
			// if there was something after last slash, check if it contains a dot and extract extension
			if (extractFileName(afterLastSlash, this->_fileName, this->_fileExtension))
			{
				// then everything before the last slash is route
				this->_route = matches[1];
			}
			else
			{
				// otherwise everything is route
				this->_route = _target;
			}
		}
	}
	// trim slashes
	this->_route = StringUtils::trimChar(this->_route, '/');
	this->_fileName = StringUtils::trimChar(this->_fileName, '/');
	std::cout << RED << "splitTarget(): Route: " << this->_route << RESET << std::endl;
	std::cout << RED << "splitTarget(): File name: " << this->_fileName << RESET << std::endl;
	std::cout << RED << "splitTarget(): File extension: " << this->_fileExtension << RESET << std::endl;
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
	std::cout << "location root: " << this->_location.getLocationRoot() << std::endl;
	std::cout << "route: " << this->_route << std::endl;

	std::string path = StringUtils::joinPath(this->_location.getLocationRoot(), this->_route, this->_fileName);

	std::cout << "Joined path: " << path << std::endl;

	if (FileSystemUtils::isDir(path))
	{
		std::string dirPath = path;
		if (this->_location.getDirectoryListing())
		{
			std::cout << RED << "Directory listing" << RESET << std::endl;
			// serve directory listing
			this->_body = BinaryData::getDirectoryListingPage(dirPath);
			this->_statusCode = HttpStatusCode::OK;
		}
		else if (!this->_location.getDefaultFile().empty())
		{
			std::cout << RED << "Getting index file" << RESET << std::endl;
			// check if this should be target or some location property
			this->_body = BinaryData::getFileData(dirPath + this->_location.getDefaultFile());
			this->_statusCode = HttpStatusCode::OK;
			// server index file
		}
		else
		{
			std::cout << RED << "Not Directory listing or Index file" << RESET << std::endl;
			// 403 Forbidden
		}
	}
	else
	{
		if (FileSystemUtils::isFile(path))
		{
			std::cout << RED << "Getting file" << RESET << std::endl;
			this->_body = BinaryData::getFileData(path);
			this->_statusCode = HttpStatusCode::OK;
			// serve file
		}
		else
		{
			std::cout << RED << "Not a file, some weird shit" << RESET << std::endl;
			this->_statusCode = HttpStatusCode::NOT_FOUND;
			prepareErrorResponse();
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

void Response::handleAlias()
{
	std::string alias = _location.getLocationAlias();
	if (!alias.empty())
	{
		std::cout << RED << "Alias: " << _location.getLocationAlias() << RESET << std::endl;
		std::cout << RED << "route: " << this->_route << std::endl;
		std::cout << RED << "location route: " << _location.getLocationRoute() << std::endl;
		StringUtils::replaceFirstOccurrence(this->_route, _location.getLocationRoute(), _location.getLocationAlias());
		_location.setLocationRoot("");
		std::cout << "route after alias replacement: " << this->_route << std::endl;
	}
}

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod(), request.getTarget(), request.getConnection()), _request(request)
{
	(void)_request;
	this->prepareStandardHeaders();
	if (this->_statusCode != HttpStatusCode::UNDEFINED_STATUS)
	{
		std::cout << RED << "Creating response based on error" << RESET << std::endl;
		// we already know what the response will be, just need to format it
		this->prepareErrorResponse();
	}
	else
	{
		splitTarget();
		// try to match location
		try
		{
			this->_location = _config.getMatchingLocation(_route);
			std::cout << RED << "Location '" << _route << "' found" << RESET << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << RED << "Didn't find location for: " << _route << "' found" << RESET << std::endl;
			this->_statusCode = HttpStatusCode::NOT_FOUND;
			this->prepareErrorResponse();
			return;
		}
		// handle redirection
		if (isRedirect())
		{
			std::cout << RED << "Redirect" << RESET << std::endl;
			this->prepareRedirectResponse();
			return;
		}
		// handle aliases - should override root
		handleAlias();
		// make sure target exists
		if (!targetFound())
		{
			std::cout << RED << "Target not found" << RESET << std::endl;
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