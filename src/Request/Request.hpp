#ifndef REQUEST_HPP
#define REQUEST_HPP

// TODO: check includes
#include "../defines.hpp"
#include <string>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <limits>
#include "HttpMessage.hpp"

class Request : public HttpMessage
{
private:
	// STRUCTS

	struct RequestLine
	{
		std::string method;
		std::string requestTarget;
		std::string HTTPVersionMajor;
		/*
		When a major version of HTTP does not define any minor versions, the minor version "0" is implied. A recipient that receives a message with a major version number that it implements and a minor version number higher than what it implements SHOULD process the message as if it were in the highest minor version within that major version to which the recipient is conformant.
		*/
		// What about version 1.0 - do we need to ask to upgrade? probably yes
	};

	// PROPERTIES

	// Helper properties for initial data read
	std::unordered_map<std::string, std::string> _headerLines;
	RequestLine _requestLine;

	// Properties unique to Request
	bool _bodyExpected;
	std::string _userAgent;
	std::string _host;
	int _port;
	std::string _transferEncoding;
	// OPTIONAL: handle Expect header

	// METHODS

	// helpers
	bool isDigitsOnly(const std::string &str) const;
	size_t strToSizeT(const std::string &str) const;
	std::string removeComments(const std::string &input) const;
	std::vector<std::string> splitByCRLF(const std::string &input) const;
	std::vector<std::string> splitByDelimiter(const std::string &input, const std::string &delimiter) const;
	std::vector<std::string> splitCommaSeparatedList(const std::string &input) const;
	// initial data reading
	void extractRequestLine(const std::string &requestLine);
	void extractHeaderLine(const std::string &headerLine);
	// parsing
	int parseVersion();
	HttpMethod matchValidMethod();
	HttpMethod parseMethod();
	void validateMethod();

	void parseRequestLine();
	void parseHost();
	void parseContentLength();
	void parseTransferEncoding();
	void parseUserAgent();
	void parseHeaders();
	void parseConnection();

	// main function
	void processRequest(const std::string &requestLineAndHeaders);

public:
	Request(const std::string &requestLineAndHeaders, const ConfigData &config);
	Request(HttpStatusCode statusCode, const ConfigData &config);

	// SETTERS

	void appendToBody(const std::vector<std::byte> &newBodyChunk);

	// GETTERS

	std::string getHost() const;
	std::string getUserAgent() const;
	bool isBodyExpected() const;
	std::string getTransferEncoding() const;

	// EXCEPTIONS

	class BadRequestException : public std::exception
	{
	};

	// DEBUG
	void printRequestProperties() const;
};

#endif
