#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../HttpMessage/HttpMessage.hpp"
#include "../DefaultErrorPage/DefaultErrorPage.hpp"
#include "../DirectoryListingPage/DirectoryListingPage.hpp"
#include "../StringUtils/StringUtils.hpp"
#include "../FileSystemUtils/FileSystemUtils.hpp"
#include "../config_parser/Location.hpp"

#include <string>
#include <chrono>
#include <vector>
// TODO: check imports below
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iomanip>

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	std::string _serverHeader;
	std::chrono::system_clock::time_point _lastModified;

	Request const &_request;

	std::string formatDate() const;
	std::string getHeader() const;
	std::string formatStatusLine() const;
	std::string formatHeader() const;
	std::string formatStatusCodeMessage() const;
	std::string formatConnection() const;
	std::string formatContentType() const;

	void setDateToCurrent();
	void prepareResponse();
	void prepareErrorResponse();
	void prepareStandardHeaders();

	void checkForRedirect();
	void validateTarget();
	bool isCGI();
	void executeCGI();
	void handlePost();
	void handleGet();
	void handleHead();
	void handleDelete();

public:
	Response(const Request &request);

	std::vector<std::byte> formatResponse() const;
	void printResponseProperties() const;
};

#endif
