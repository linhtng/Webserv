#include "Response.hpp"

// DEBUGGING FUNCTIONS

void Response::printResponseProperties() const
{
	std::cout << "Response properties:" << std::endl;
	std::cout << "Server header: " << this->_serverHeader << std::endl;
	std::cout << "Location header: " << this->_locationHeader << std::endl;
	std::cout << "Upgrade header: " << this->_upgradeHeader << std::endl;
	std::cout << "Content length: " << this->_contentLength << std::endl;
	std::cout << "Content type: " << this->_contentType << std::endl;
	std::cout << "Connection: " << this->formatConnection() << std::endl;
	std::cout << "Date: " << this->formatDate() << std::endl;
	std::cout << "Status code: " << this->formatStatusCodeMessage() << std::endl;
	std::cout << "Status line: " << this->formatStatusLine() << std::endl;
	std::cout << "Header: " << this->formatHeader() << std::endl;
	std::cout << "Body size: " << this->_body.size() << std::endl;
	std::cout << "Critical error: " << this->_criticalError << std::endl;
	std::cout << "Redirection route: " << this->_redirectionRoute << std::endl;
	std::cout << "Location path: " << this->_locationPath << std::endl;
	std::cout << "Actual location path: " << this->_actualLocationPath << std::endl;
	std::cout << "Path after location: " << this->_pathAfterLocation << std::endl;
	std::cout << "File name: " << this->_fileName << std::endl;
	std::cout << "File extension: " << this->_fileExtension << std::endl;
	std::cout << "Query params: " << this->_queryParams << std::endl;
	std::cout << "Method: " << HttpUtils::_httpMethodToStr.at(this->_method) << std::endl;
	std::cout << "HTTP version: " << this->_httpVersionMajor << "." << this->_httpVersionMinor << std::endl;
	std::cout << "Boundary: " << this->_boundary << std::endl;
	std::cout << "Parts size: " << this->_parts.size() << std::endl;
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
	try
	{
		return HttpUtils::_contentTypeStrings.at(this->_contentType);
	}
	catch (const std::out_of_range &e)
	{
		return "text/html";
	}
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
	if (this->_criticalError)
	{
		std::string errorMessage = CRITICAL_ERROR_RESPONSE;
		for (char ch : errorMessage)
		{
			response.push_back(static_cast<std::byte>(ch));
		}
	}
	else
	{
		std::string formattedHeader = this->formatHeader();
		for (char ch : formattedHeader)
		{
			response.push_back(static_cast<std::byte>(ch));
		}
		std::vector<std::byte> body = this->_body;
		response.insert(response.end(), body.begin(), body.end());
	}
	return response;
}

// RESPONSE PREPARATION

void Response::setDateToCurrent()
{
	this->_date = std::chrono::system_clock::now();
}

bool Response::getConfiguredErrorPage()
{
	std::unordered_map<int, std::string> errorPages = this->_config.getErrorPages();
	try
	{
		std::string errorPagePath = errorPages.at(this->_statusCode);
		this->_body = BinaryData::getFileData(StringUtils::trim(errorPagePath));
	}
	catch (const std::out_of_range &e)
	{
		Logger::log(INFO, SERVER, "Error page not found");
	}
	catch (const std::exception &e)
	{
		Logger::log(ERROR, SERVER, "Internal error trying to get configured error page: %s", e.what());
	}
	return false;
}

void Response::prepareErrorResponse()
{
	prepareStandardHeaders();
	Logger::log(DEBUG, SERVER, "getting error page for status code: %d", this->_statusCode);
	try
	{
		if (!getConfiguredErrorPage())
		{
			this->_body = BinaryData::getErrorPage(this->_statusCode);
		}
	}
	catch (const std::exception &e)
	{
		Logger::log(ERROR, SERVER, "Error preparing error response: %s", e.what());
		this->_criticalError = true;
		return;
	}
	this->_contentLength = this->_body.size();
	this->_contentType = ContentType::TEXT_HTML;
	// this->_connection = ConnectionValue::CLOSE;
	if (this->_statusCode == HttpStatusCode::UPGRADE_REQUIRED)
	{
		this->_connection = ConnectionValue::UPGRADE;
		this->_upgradeHeader = "HTTP/1.1";
	}
}

void Response::prepareStandardHeaders()
{
	this->setDateToCurrent();
	this->_serverHeader = SERVER_SOFTWARE;
}

void Response::prepareRedirectResponse()
{
	Logger::log(INFO, SERVER, "Redirecting to: %s", this->_redirectionRoute.c_str());
	if (this->_method == HttpMethod::POST)
	{
		this->_statusCode = HttpStatusCode::PERMANENT_REDIRECT;
	}
	else
	{
		this->_statusCode = HttpStatusCode::MOVED_PERMANENTLY;
	}
	this->_contentType = ContentType::TEXT_PLAIN;
	this->_locationHeader = '/' + this->_redirectionRoute;
}

bool Response::isRedirect()
{
	this->_redirectionRoute = this->_location.getRedirectionRoute();
	return !this->_location.getRedirectionIsEmpty();
}

bool Response::targetFound()
{
	std::string fullPathNotTrimmed = StringUtils::joinPath(this->_actualLocationPath, this->_pathAfterLocation, this->_fileName);
	std::string fullPath = StringUtils::trimChar(fullPathNotTrimmed, '/');
	if (!FileSystemUtils::pathExists(fullPath))
	{
		Logger::log(DEBUG, SERVER, "Target %s not found", fullPath.c_str());
		this->_statusCode = HttpStatusCode::NOT_FOUND;
		return false;
	}
	else
	{
		Logger::log(DEBUG, SERVER, "Target %s found", fullPath.c_str());
		return true;
	}
}

//  probably would be nice to move this to utils
bool Response::extractFileNameAndQuery(const std::string &fileNameWithQuery)
{
	// split file extension+query from filename
	size_t dotPos = fileNameWithQuery.find_last_of('.');
	if (dotPos == std::string::npos)
	{
		// if there is no dot, it's not a file
		return false;
	}
	std::string extensionWithQuery = fileNameWithQuery.substr(dotPos + 1);
	// split query from extension if it exists
	size_t queryPos = extensionWithQuery.find("?");
	if (queryPos == std::string::npos)
	{
		this->_fileExtension = extensionWithQuery;
	}
	else
	{
		this->_queryParams = extensionWithQuery.substr(queryPos + 1);
		this->_fileExtension = extensionWithQuery.substr(0, queryPos);
	}
	// compose full file name
	this->_fileName = fileNameWithQuery.substr(0, dotPos + 1) + this->_fileExtension;
	return true;
}

void Response::splitTarget()
{
	std::string trimmedTarget = StringUtils::trimChar(this->_target, '/');
	// first separate part that is same as location
	this->_locationPath = StringUtils::trimChar(this->_location.getLocationRoute(), '/');
	// actualLocationPath for now same as location path, will be changed later if alias or root is present
	this->_actualLocationPath = this->_locationPath;
	// then separate the rest
	this->_pathAfterLocation = StringUtils::trimChar(StringUtils::removePrefix(trimmedTarget, this->_locationPath), '/');
	if (this->_pathAfterLocation.empty())
	{
		// if there's nothing after location, we're done
		return;
	}
	// split the path after location
	std::vector<std::string> pathAfterLocationSplit = StringUtils::splitByDelimiter(this->_pathAfterLocation, "/");
	// last part - file name with query (or just last directory if no file name)
	std::string potentialFilenameWithQuery = pathAfterLocationSplit.back();
	if (extractFileNameAndQuery(potentialFilenameWithQuery))
	{
		// if file name was extracted, remove it from _pathAfterLocation
		this->_pathAfterLocation = StringUtils::trimChar(StringUtils::removeSuffix(this->_pathAfterLocation, potentialFilenameWithQuery), '/');
	}
}

bool Response::isCGI()
{
	std::vector<std::string> cgiExtensions;
	for (auto &extenExecutor : this->_config.getCgiExtenExecutorMap())
	{
		cgiExtensions.push_back(extenExecutor.first);
	}
	if (cgiExtensions.empty())
	{
		Logger::log(DEBUG, SERVER, "No CGI extensions found in the config");
		return false;
	}
	if (std::find(cgiExtensions.begin(), cgiExtensions.end(), "." + this->_fileExtension) != cgiExtensions.end())
	{
		Logger::log(DEBUG, SERVER, "CGI script detected");
		return true;
	}
	Logger::log(DEBUG, SERVER, "Not a CGI script");
	return false;
}

// LINH_CGI
// forms a response based on CGI output
//  setCGIResponse (cgiOutput, cgiExitStatus) // or just take cgiHandler class instance

void Response::executeCGI()
{
	Logger::log(DEBUG, SERVER, "Executing CGI script: %s", this->_fileName.c_str());
	// reject CGI if method is not GET or POST
	if (this->_method != HttpMethod::GET && this->_method != HttpMethod::POST)
	{
		this->_statusCode = HttpStatusCode::METHOD_NOT_ALLOWED;
		throw ClientException("CGI script can only be executed with GET or POST method");
	}

	// TODO: either remove comments or return to try-catch
	// try
	// {
	std::unordered_map<std::string, std::string> cgiParams;
	cgiParams["fileName"] = this->_fileName;
	cgiParams["fileExtension"] = "." + this->_fileExtension;
	cgiParams["queryParams"] = this->_queryParams;

	CgiHandler cgiHandler(_request, cgiParams);
	try
	{
		cgiHandler.initializeCgi(_request, cgiParams);
		cgiHandler.createCgiProcess();
		this->_body = BinaryData::strToVectorByte(cgiHandler.getCgiOutput());
		this->_contentType = ContentType::TEXT_PLAIN;
		this->_statusCode = cgiHandler.getCgiExitStatus();
	}
	catch (const std::exception &e)
	{
		Logger::log(e_log_level::ERROR, CLIENT, "Error executing CGI script: %s, server error", e.what());
		HttpStatusCode cgiExitStatus = cgiHandler.getCgiExitStatus();
		if (cgiExitStatus == HttpStatusCode::UNDEFINED_STATUS)
		{
			this->_statusCode = HttpStatusCode::INTERNAL_SERVER_ERROR;
		}
		else
		{
			this->_statusCode = cgiExitStatus;
		}
		throw ServerException("Error executing CGI script");
	}
	// }
	// catch (const std::exception &e)
	// {
	// 	Logger::log(e_log_level::ERROR, CLIENT, "Error executing CGI script: %s, server error", e.what());
	// 	this->_statusCode = HttpStatusCode::INTERNAL_SERVER_ERROR;
	// 	throw ServerException("Error executing CGI script, constructor failed");
	// }
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
	std::string firstAlwaysFormData = StringUtils::trim(split[0]);
	if (firstAlwaysFormData != "form-data")
	{
		throw ClientException("Content-Disposition header does not contain form-data directive");
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
			throw ClientException("Invaild Content-Disposition header params");
		}
		std::string paramName = StringUtils::trim(paramsSplit[0]);
		std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::tolower);
		params[paramName] = StringUtils::trimChar(StringUtils::trim(paramsSplit[1]), '"');
		// print key value pair for debug
		Logger::log(DEBUG, SERVER, "Key: %s Value: %s", paramName.c_str(), params[paramName].c_str());
	}
	// TODO: move this to the initial multipart processing part later
	// if no filename, no upload
	auto filenameIt = params.find("filename");
	if (filenameIt == params.end())
	{
		throw ClientException("No filename in Content-Disposition header");
	}
	std::string fileName = filenameIt->second;
	if (fileName.empty())
	{
		throw ClientException("Empty filename in Content-Disposition header");
	}
	// TODO: fgure out the root/alias situation
	std::string savePath = StringUtils::joinPath(this->_actualLocationPath, this->_pathAfterLocation, this->_location.getSaveDir());
	Logger::log(DEBUG, SERVER, "Saving file to: %s", savePath.c_str());
	// save the file
	FileSystemUtils::saveFile(savePath, fileName, part.body);
}

void Response::handlePost()
{
	// if it's not multipart form, just return back the data sent by client
	if (this->_request.getContentType() != ContentType::MULTIPART_FORM_DATA)
	{
		this->_body = this->_request.getBody();
		this->_contentType = this->_request.getContentType();
		this->_statusCode = HttpStatusCode::CREATED;
		return;
	}
	// check if upload is allowed
	if (this->_location.getSaveDirIsEmpty())
	{
		this->_statusCode = HttpStatusCode::FORBIDDEN;
		throw ClientException("Upload not allowed, no save_dir specified in location");
	}
	// iterate throu parts and save them to files
	for (size_t i = 0; i < this->_parts.size(); i++)
	{
		postMultipartDataPart(this->_parts[i]);
		// TODO: error handling
	}
	// set the Location header to contain path to the uploads directory
	this->_locationHeader = '/' + this->_location.getSaveDir();
	this->_body = BinaryData::strToVectorByte("File uploaded successfully");
	this->_contentType = ContentType::TEXT_PLAIN;
	this->_statusCode = HttpStatusCode::CREATED;
}

void Response::handleGet()
{
	std::string path = StringUtils::joinPath(this->_actualLocationPath, this->_pathAfterLocation, this->_fileName);
	std::string userPath = StringUtils::joinPath(this->_locationPath, this->_pathAfterLocation);

	if (FileSystemUtils::isDir(path))
	{
		std::string dirPath = StringUtils::trimChar(path, '/');
		Logger::log(DEBUG, SERVER, "GET directory: %s", path.c_str());
		if (this->_location.getDirectoryListing())
		{
			Logger::log(DEBUG, SERVER, "Serving directory listing: %s", dirPath.c_str());
			this->_body = BinaryData::getDirectoryListingPage(this->_locationPath, this->_actualLocationPath, this->_pathAfterLocation);
			this->_statusCode = HttpStatusCode::OK;
			this->_contentType = ContentType::TEXT_HTML;
		}
		else if (!this->_location.getDefaultFile().empty())
		{
			// check if this should be target or some location property
			Logger::log(DEBUG, SERVER, "Serving index file: %s", this->_location.getDefaultFile().c_str());
			this->_body = BinaryData::getFileData(StringUtils::joinPath(dirPath, this->_location.getDefaultFile()));
			this->_statusCode = HttpStatusCode::OK;
			this->_contentType = ContentType::TEXT_HTML;
		}
		else
		{
			this->_statusCode = HttpStatusCode::FORBIDDEN;
			throw ClientException("No directory listing or index file specified in location, forbidden");
		}
	}
	else
	{
		if (FileSystemUtils::isFile(path))
		{
			Logger::log(DEBUG, SERVER, "Serving file: %s", path.c_str());
			this->_body = BinaryData::getFileData(path);
			this->_statusCode = HttpStatusCode::OK;
		}
		else
		{
			this->_statusCode = HttpStatusCode::NOT_FOUND;
			throw ClientException("Not a file or a directory");
		}
	}
}

void Response::handleHead()
{
	handleGet();
	this->_contentLength = this->_body.size();
	Logger::log(DEBUG, SERVER, "Set HEAD content length: %d", this->_contentLength);
	this->_body.clear();
}

void Response::handleDelete()
{
	Logger::log(DEBUG, SERVER, "DELETE request");
	// if there's no upload dir or we're not in the upload dir, reject
	std::string saveDir = this->_location.getSaveDir();
	if (this->_location.getSaveDirIsEmpty() || saveDir != this->_pathAfterLocation)
	{
		this->_statusCode = HttpStatusCode::FORBIDDEN;
		throw ClientException("Delete not allowed, no save_dir specified in location or target is not in save_dir");
	}
	if (this->_fileName.empty())
	{
		this->_statusCode = HttpStatusCode::FORBIDDEN;
		throw ClientException("Delete is only allowed on files");
	}
	std::string path = StringUtils::joinPath(this->_actualLocationPath, this->_pathAfterLocation, this->_fileName);
	// if it's not a valid file, reject
	if (!FileSystemUtils::isFile(path))
	{
		this->_statusCode = HttpStatusCode::NOT_FOUND;
		throw ClientException("File not found");
	}
	else
	{
		FileSystemUtils::deleteFile(path);
		this->_statusCode = HttpStatusCode::NO_CONTENT;
		this->_body = BinaryData::strToVectorByte("File deleted successfully");
		this->_contentType = ContentType::TEXT_PLAIN;
		// TODO: check if there's any special headers or body that it needs to return?
	}
}

void Response::handleRootAndAlias()
{
	std::string alias = _location.getLocationAlias();
	std::string root = _location.getLocationRoot();
	if (!_location.getAliasIsEmpty())
	{
		this->_actualLocationPath = alias;
	}
	if (!_location.getRootIsEmpty())
	{
		this->_actualLocationPath = StringUtils::joinPath(this->_location.getLocationRoot(), this->_actualLocationPath);
	}
}

void Response::processMultipartDataPartHeaders(MultipartDataPart &dataPart, std::string headersString)
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

void Response::processMultipartDataPart(std::vector<std::byte> part)
{
	std::cout << "\nprocessMultipartDataPart():" << std::endl;
	for (auto ch : part)
	{
		std::cout << static_cast<char>(ch);
	}
	MultipartDataPart dataPart;

	// fnd the end of headers and process them, they can be processed as a string
	std::string partString(reinterpret_cast<const char *>(part.data()), part.size());
	auto crlfPos = partString.find(CRLF CRLF);
	if (crlfPos == std::string::npos)
	{
		throw ClientException("No CRLF CRLF found in multipart data part to separte headers");
	}
	// substring until the first CRLF CRLF
	std::string headersString = partString.substr(0, crlfPos);
	processMultipartDataPartHeaders(dataPart, headersString);

	// the rest is the body, it needs to be processed as binary
	dataPart.body = std::vector<std::byte>(part.begin() + crlfPos + 4, part.end());

	this->_parts.push_back(dataPart);
}

void Response::processMultipartData()
{
	if (this->_request.getContentType() != ContentType::MULTIPART_FORM_DATA)
	{
		return;
	}
	std::vector<std::byte> requestBody = this->_request.getBody();
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
		throw ClientException("First boundary not found in multipart data");
	}

	// Skip the delimiter
	partStart += delimiterBytes.size();

	// find endDelimiter

	auto endDelimiterIt = std::search(partStart, messageBody.end(), endDelimiterBytes.begin(), endDelimiterBytes.end());
	if (endDelimiterIt == messageBody.end())
	{
		throw ClientException("End boundary not found in multipart data");
	}

	while (partStart != messageBody.end())
	{
		// Find the end of the current part
		auto partEnd = std::search(partStart, messageBody.end(), delimiterBytes.begin(), delimiterBytes.end());
		// TODO: finish the whole crlf condition thing
		if (partEnd == messageBody.end() || partEnd - partStart < 4)
		{
			throw ClientException("Invalid multipart data format, no next delimiter found or too short part");
		}
		// Extract the current part
		std::vector<std::byte> part(partStart + 2, partEnd - 2);
		processMultipartDataPart(part);
		// if this was the last part, stop
		if (partEnd == endDelimiterIt)
		{
			break;
		}
		// If not the end, move to the start of the next part
		partStart = partEnd + delimiterBytes.size();
	}
	Logger::log(DEBUG, SERVER, "Processed multipart data, parts detected: %d", this->_parts.size());
	if (this->_parts.size() == 0)
	{
		throw ClientException("No parts found in multipart data");
	}
	/* std::cout << "Parsed parts:" << std::endl;
	for (auto &part : this->_parts)
	{
		for (auto &[key, value] : part.headers)
		{
			std::cout << key << ": " << value << std::endl;
		}
		std::cout << "Body:" << std::endl;
		for (auto ch : part.body)
			std::cout << static_cast<char>(ch);
	} */
}

bool Response::methodAllowed()
{
	std::unordered_set<HttpMethod> allowedMethods = this->_location.getAcceptedMethods();
	if (std::find(allowedMethods.begin(), allowedMethods.end(), this->_method) == allowedMethods.end())
	{
		return false;
	}
	return true;
}

void Response::prepareResponse()
{
	// If error is already known, just go straight to error page forming
	if (this->_statusCode != HttpStatusCode::UNDEFINED_STATUS)
	{
		throw ServerException("Error status code already set, skipping response preparation");
	}
	// Process multipart form if it exists
	try
	{
		processMultipartData();
	}
	catch (const std::exception &e)
	{
		throw ClientException("Error processing multipart data");
	}
	// Prepare headers that are standard for every response
	prepareStandardHeaders();

	// Try to match location
	try
	{
		this->_location = _config.getMatchingLocation(this->_target);
	}
	catch (const std::exception &e)
	{
		this->_statusCode = HttpStatusCode::NOT_FOUND;
		throw ClientException("No matching location found for route");
	}
	// Handle redirections
	if (isRedirect())
	{
		prepareRedirectResponse();
		return;
	}

	// Process target path
	splitTarget();
	handleRootAndAlias();

	Logger::log(DEBUG, SERVER, "Location path: %s", this->_locationPath.c_str());
	Logger::log(DEBUG, SERVER, "Actual location path: %s", this->_actualLocationPath.c_str());
	Logger::log(DEBUG, SERVER, "Path after location: %s", this->_pathAfterLocation.c_str());
	Logger::log(DEBUG, SERVER, "File name: %s", this->_fileName.c_str());
	Logger::log(DEBUG, SERVER, "File extension: %s", this->_fileExtension.c_str());
	Logger::log(DEBUG, SERVER, "Query params: %s", this->_queryParams.c_str());

	// Refuse if method not allowed
	if (!methodAllowed())
	{
		this->_statusCode = HttpStatusCode::METHOD_NOT_ALLOWED;
		throw ClientException("Method not allowed");
	}
	// CGI handling
	if (isCGI())
	{
		Logger::log(e_log_level::INFO, CLIENT, "CGI script detected");
		executeCGI();
		// LINH_CGI
		// this->isCgi = true;
	}
	else
	{
		// Make sure target exists
		if (!targetFound())
		{
			this->_statusCode = HttpStatusCode::NOT_FOUND;
			throw ClientException("Target not found");
		}

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
			this->_statusCode = HttpStatusCode::METHOD_NOT_ALLOWED;
			throw ClientException("Method not allowed");
		}
	}
	// Set content length
	if (this->_method != HttpMethod::HEAD)
	{
		this->_contentLength = this->_body.size();
	}
}

// CONSTRUCTOR

Response::Response(const Request &request) : HttpMessage(request.getConfig(), request.getStatusCode(), request.getMethod(), request.getTarget(), request.getConnection(), request.getHttpVersionMajor(), request.getHttpVersionMinor(), request.getBoundary(), request.getCriticalError()), _request(request)
{
	try
	{
		prepareResponse();
	}
	catch (const ClientException &e)
	{
		Logger::log(ERROR, CLIENT, "Client error: %s", e.what());
		try
		{
			if (this->_statusCode == HttpStatusCode::UNDEFINED_STATUS)
			{
				this->_statusCode = HttpStatusCode::BAD_REQUEST;
			}
			prepareErrorResponse();
		}
		catch (const std::exception &e)
		{
			this->_criticalError = true;
		}
	}
	catch (const std::exception &e)
	{
		Logger::log(ERROR, SERVER, "Server error: %s", e.what());
		try
		{
			if (this->_statusCode == HttpStatusCode::UNDEFINED_STATUS)
			{
				this->_statusCode = HttpStatusCode::INTERNAL_SERVER_ERROR;
			}
			prepareErrorResponse();
		}
		catch (const std::exception &e)
		{
			this->_criticalError = true;
		}
	}
}
