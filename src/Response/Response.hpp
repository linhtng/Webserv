#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../defines.hpp"
#include "../Request/Request.hpp"
#include "../HttpMessage/HttpMessage.hpp"
#include "../StringUtils/StringUtils.hpp"
#include "../FileSystemUtils/FileSystemUtils.hpp"
#include "../config_parser/Location.hpp"
#include "../BinaryData/BinaryData.hpp"
#include "../HttpUtils/HttpUtils.hpp"
#include "../Logger/Logger.hpp"
#include "../CgiHandler/CgiHandler.hpp"

#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
// TODO: check imports below

#include <iomanip> //TODO: check header for put_time

class Request;

class HttpMessage;

class Response : public HttpMessage
{
private:
	struct MultipartDataPart
	{
		std::map<std::string, std::string> headers;
		std::vector<std::byte> body;
	};

	std::string _serverHeader;
	std::string _locationHeader;
	std::string _upgradeHeader;

	std::vector<MultipartDataPart> _parts;
	Request const &_request;
	Location _location;
	std::string _redirectionRoute;
	// these will be populated by splitTarget()
	std::string _locationPath;		 // location route from config
	std::string _actualLocationPath; // location route from config with root/alias logic applied
	std::string _pathAfterLocation;	 // path after location route
	std::string _fileName;			 // filename + extension
	std::string _fileExtension;		 // extension
	std::string _queryParams;		 // query string

	bool extractFileNameAndQuery(const std::string &fileName);
	std::string formatDate() const;
	std::string formatStatusLine() const;
	std::string formatHeader() const;
	std::string formatStatusCodeMessage() const;
	std::string formatConnection() const;
	std::string formatContentType() const;

	void splitTarget();
	bool getConfiguredErrorPage();
	void setDateToCurrent();
	void prepareErrorResponse();
	void prepareStandardHeaders();
	void prepareRedirectResponse();
	void processMultipartData();
	void processMultipartDataPart(std::vector<std::byte> part);
	void postMultipartDataPart(const MultipartDataPart &part);
	void processMultipartDataPartHeaders(MultipartDataPart &dataPart, std::string headersString);
	bool isRedirect(); // consts?
	void handleRootAndAlias();
	bool targetFound();
	bool isCGI();
	void executeCGI();
	void handlePost();
	void handleGet();
	void handleHead();
	void handleDelete();
	void prepareResponse();

public:
	Response(const Request &request);

	std::vector<std::byte> formatResponse() const;
	void printResponseProperties() const;

	class ClientException : public std::exception
	{
	private:
		const char *message;

	public:
		ClientException(const char *msg) : message(msg) {}
		const char *what() const throw() override
		{
			return message;
		}
	};

	class ServerException : public std::exception
	{
	private:
		const char *message;

	public:
		ServerException(const char *msg) : message(msg) {}
		const char *what() const throw() override
		{
			return message;
		}
	};
};

#endif
