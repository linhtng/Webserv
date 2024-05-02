#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../HttpMessage/HttpMessage.hpp"
#include "../StringUtils/StringUtils.hpp"
#include "../FileSystemUtils/FileSystemUtils.hpp"
#include "../config_parser/Location.hpp"
#include "../BinaryData/BinaryData.hpp"

#include <string>
#include <chrono>
#include <vector>
// TODO: check imports below

#include <iomanip> //TODO: check header for put_time

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	std::string _serverHeader;
	std::string _locationHeader;

	Request const &_request;
	Location _location;
	std::string _redirectionRoute;
	std::string _route;
	std::string _fileName;
	std::string _fileExtension;

	void splitTarget();
	bool extractFileName(const std::string &fileName, std::string &name, std::string &extension);
	std::string formatDate() const;
	std::string formatStatusLine() const;
	std::string formatHeader() const;
	std::string formatStatusCodeMessage() const;
	std::string formatConnection() const;
	std::string formatContentType() const;

	void setDateToCurrent();
	void prepareResponse();
	void prepareErrorResponse();
	void prepareStandardHeaders();
	void prepareRedirectResponse();

	bool isRedirect(); // consts?
	void handleAlias();
	bool targetFound();
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
