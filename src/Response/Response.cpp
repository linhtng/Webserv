#include "Response.hpp"

// DEBUGGING FUNCTIONS

void Response::printResponseProperties() const
{
	std::cout << "Response properties:" << std::endl;
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
	std::string errorMessage = DEFAULT_ERROR_MESSAGE;
	try
	{
		errorMessage = HttpUtils::_statusCodeMessages.at(this->_statusCode);
	}
	catch (const std::out_of_range &e)
	{
	}
	return (std::to_string(this->_statusCode) + " " + errorMessage);
}

std::string Response::formatConnection() const
{
	if (this->_connection == ConnectionValue::CLOSE)
	{
		return "close";
	}
	else if (this->_connection == ConnectionValue::UPGRADE)
	{
		return "Upgrade";
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
	header += "Server: " SERVER_SOFTWARE CRLF;
	header += "Content-Length: " + std::to_string(this->_body.size()) + CRLF;
	if (!this->_upgradeHeader.empty())
	{
		header += "Upgrade: " + this->_upgradeHeader + CRLF;
	}
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
	std::string formattedHeader = this->formatHeader();
	for (char ch : formattedHeader)
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	std::vector<std::byte> body = this->_body;
	std::cout << RED << "Formatted response" << RESET << std::endl;
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
}

void Response::prepareErrorResponse()
{
	this->_body = BinaryData::getErrorPage(this->_statusCode);
	this->_contentLength = this->_body.size();
	if (this->_statusCode == HttpStatusCode::UPGRADE_REQUIRED)
	{
		this->_connection = ConnectionValue::UPGRADE;
		this->_upgradeHeader = "HTTP/1.1";
	}
	// TODO: ? set Content-Type to http
}

void Response::prepareStandardHeaders()
{
	this->setDateToCurrent();
	this->_serverHeader = SERVER_SOFTWARE;
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
	this->_locationHeader = '/' + this->_redirectionRoute;
}

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

void Response::postMultipartDataPart(const MultipartDataPart &part)
{
	auto it = part.headers.find("content-disposition");
	if (it == part.headers.end())
	{
		// if no content-disposition, reject
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		// prepare error response?
		return;
	}
	// check if filename param exists in content-disposition header
	// split into params
	/* A multipart/form-data body requires a Content-Disposition header to provide information for each subpart of the form (e.g. for every form field and any files that are part of field data). The first directive is always form-data, and the header must also include a name parameter to identify the relevant field. Additional directives are case-insensitive and have arguments that use quoted-string syntax after the '=' sign. Multiple parameters are separated by a semicolon (';'). */
	std::vector<std::string> split = StringUtils::splitByDelimiter(it->second, ";");
	// first part is always form-data
	if (split.size() < 1)
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		// prepareErrorResponse();
		return;
	}
	std::string firstAlwaysFormData = StringUtils::trim(split[0]);
	if (firstAlwaysFormData != "form-data")
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		// prepareErrorResponse();
		return;
	}
	// extract params liek name and filename

	// this is code duplication, TODO: fix
	std::unordered_map<std::string, std::string> params;
	for (size_t i = 1; i < split.size(); ++i)
	{
		std::vector<std::string> paramsSplit = StringUtils::splitByDelimiter(split[i], "=");
		if (paramsSplit.size() != 2)
		{
			// if no =, reject
			this->_statusCode = HttpStatusCode::BAD_REQUEST;
			// prepareErrorResponse();
			return;
		}
		std::string paramName = StringUtils::trim(paramsSplit[0]);
		std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::tolower);
		params[paramName] = StringUtils::trimChar(StringUtils::trim(paramsSplit[1]), '"');
		// print key value pair for debug
		std::cout << RED << "Key: " << paramName << " Value: " << params[paramName] << RESET << std::endl;
	}
	// TODO: move this to the initial multipart processing part later
	// if no filename, no upload
	auto filenameIt = params.find("filename");
	if (filenameIt == params.end())
	{
		// handle non-upload ones
		// prepareErrorResponse();
		return;
	}
	std::string fileName = filenameIt->second;
	if (fileName.empty())
	{
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		prepareErrorResponse();
		return;
	}
	// TODO: fgure out the root/alias situation
	std::string savePath = StringUtils::joinPath(this->_location.getLocationRoot(), this->_location.getLocationAlias(), this->_location.getSaveDir());
	std::cout << RED << "Saving file to: " << savePath << RESET << std::endl;
	// save the file
	FileSystemUtils::saveFile(savePath, fileName, part.body);
}

void Response::handlePost()
{
	// check if upload is allowed
	if (this->_location.getSaveDir().empty())
	{
		this->_statusCode = HttpStatusCode::FORBIDDEN;
		prepareErrorResponse();
		return;
	}
	// iterate throu parts and save them to files
	for (size_t i = 0; i < this->_parts.size(); i++)
	{
		postMultipartDataPart(this->_parts[i]);
		// TODO: error handling
	}
	// set the Location header to contain path to the uploads directory
	this->_locationHeader = '/' + this->_location.getSaveDir();
	this->_statusCode = HttpStatusCode::CREATED;
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
			this->_body = BinaryData::getFileData(StringUtils::joinPath(dirPath, this->_location.getDefaultFile()));
			this->_statusCode = HttpStatusCode::OK;
			// server index file
		}
		else
		{
			std::cout << RED << "Not Directory listing or Index file" << RESET << std::endl;
			this->_statusCode = HttpStatusCode::FORBIDDEN;
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
	handleGet();
	this->_contentLength = this->_body.size();
	this->_body.clear();
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

void Response::processMultiformDataPartHeaders(MultipartDataPart &dataPart, std::string headersString)
{
	std::stringstream partStream(headersString);
	std::string line;
	while (std::getline(partStream, line))
	{
		if (!StringUtils::trim(line).empty())
		{
			size_t pos = line.find(':');
			if (pos != std::string::npos)
			{
				std::string headerName = StringUtils::trim(line.substr(0, pos));
				std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);
				std::string headerValue = StringUtils::trim(line.substr(pos + 1));
				dataPart.headers[headerName] = headerValue;
			}
		}
	}
}

void Response::processMultiformDataPart(std::vector<std::byte> part)
{
	std::cout << "\nprocessMultiformDataPart():" << std::endl;
	for (auto ch : part)
	{
		std::cout << static_cast<char>(ch);
	}
	MultipartDataPart dataPart;

	// fnd the end of headers and process them, they can be processed as a string
	std::string partString(reinterpret_cast<const char *>(part.data()), part.size());
	auto crlfPos = partString.find(CRLF CRLF);
	// if CRLFCRLF not found, reject
	if (crlfPos == std::string::npos)
	{
		// TODO: check if this error handling makes sense
		this->_statusCode = HttpStatusCode::BAD_REQUEST;
		return;
	}
	std::cout << BLUE << "crlfPos: " << crlfPos << RESET << std::endl;
	// substring until the first CRLF CRLF
	std::string headersString = partString.substr(0, crlfPos);
	processMultiformDataPartHeaders(dataPart, headersString);

	// the rest is the body, it needs to be processed as binary
	dataPart.body = std::vector<std::byte>(part.begin() + crlfPos + 4, part.end());

	this->_parts.push_back(dataPart);
}

void Response::processMultiformData()
{
	if (this->_request.getContentType() != ContentType::MULTIPART_FORM_DATA)
	{
		return;
	}
	std::cout << GREEN << "Processing multiform data" << RESET << std::endl;
	std::cout << GREEN << "REQUEST BODY" << RESET << std::endl;
	std::vector<std::byte> requestBody = this->_request.getBody();
	for (auto ch : requestBody)
		std::cout << static_cast<char>(ch);
	const std::vector<std::byte> &messageBody = this->_request.getBody();
	const std::string delimiter = "--" + this->_boundary;
	const std::string endDelimiter = delimiter + "--";
	// convert delimiters to byte vectors for direct memory comparison
	std::vector<std::byte> delimiterBytes;
	delimiterBytes.reserve(delimiter.size());
	for (char ch : delimiter)
	{
		delimiterBytes.push_back(static_cast<std::byte>(ch));
	}
	std::vector<std::byte> endDelimiterBytes;
	endDelimiterBytes.reserve(endDelimiter.size());
	for (char ch : endDelimiter)
	{
		endDelimiterBytes.push_back(static_cast<std::byte>(ch));
	}
	// find the start of the first part
	auto partStart = std::search(messageBody.begin(), messageBody.end(), delimiterBytes.begin(), delimiterBytes.end());
	if (partStart == messageBody.end())
	{
		throw std::runtime_error("First boundary not found in multiform data");
		return;
	}

	// Skip the delimiter
	partStart += delimiterBytes.size();

	// find endDelimiter

	auto endDelimiterIt = std::search(partStart, messageBody.end(), endDelimiterBytes.begin(), endDelimiterBytes.end());
	if (endDelimiterIt == messageBody.end())
	{
		throw std::runtime_error("Wrong multiform format");
		return;
	}

	while (partStart != messageBody.end())
	{
		// Find the end of the current part
		auto partEnd = std::search(partStart, messageBody.end(), delimiterBytes.begin(), delimiterBytes.end());
		// TODO: finish the whole crlf condition thing
		if (partEnd == messageBody.end() || partEnd - partStart < 4)
		{
			throw std::runtime_error("Wrong multiform format");
			break;
		}
		// Extract the current part
		std::vector<std::byte> part(partStart + 2, partEnd - 2);
		processMultiformDataPart(part);
		// if this was the last part, stop
		if (partEnd == endDelimiterIt)
		{
			break;
		}
		// If not the end, move to the start of the next part
		partStart = partEnd + delimiterBytes.size();
	}
	std::cout << GREEN << "Processed multiform data, parts detected: " << this->_parts.size() << RESET << std::endl;
	std::cout << "Parsed parts:" << std::endl;
	for (auto &part : this->_parts)
	{
		for (auto &[key, value] : part.headers)
		{
			std::cout << key << ": " << value << std::endl;
		}
		std::cout << "Body:" << std::endl;
		for (auto ch : part.body)
			std::cout << static_cast<char>(ch);
	}
}

// CONSTRUCTOR

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod(), request.getTarget(), request.getConnection(), request.getHttpVersionMajor(), request.getHttpVersionMinor(), request.getBoundary()), _request(request)
{
	//(void)_request;
	try
	{
		processMultiformData();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Error processing multiform data: " << e.what() << RESET << std::endl;
		if (this->_statusCode == HttpStatusCode::UNDEFINED_STATUS)
		{
			this->_statusCode = HttpStatusCode::BAD_REQUEST;
		}
	}
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
		if (this->_method != HttpMethod::HEAD)
		{
			this->_contentLength = this->_body.size();
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