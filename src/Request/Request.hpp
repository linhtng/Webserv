#ifndef REQUEST_HPP
#define REQUEST_HPP

// TODO: check includes
#include "../defines.hpp"
#include <string>
#include <vector>
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
	};
	/*
	The normal procedure for parsing an HTTP message is to read the start-line
	into a structure, read each header field line into a hash table by field
	name until the empty line, and then use the parsed data to determine if
	a message body is expected.
	*/

	// PROPERTIES

	// for initial data read only
	std::unordered_map<std::string, std::string> _headerLines;
	RequestLine _requestLine;
	// values from headers
	/* HttpMethod _method;
	std::string _target;
	int _httpVersionMajor;
	size_t _contentLength;
	std::vector<std::byte> _body; */
	// status values
	RequestStatus _status; // maybe not needed and status code is enough? so getStatus would just return bool based on status code value being UNDEFUNED or not
	/* HttpStatusCode _statusCode;
	bool _chunked; */
	bool _bodyExpected;
	std::string _userAgent;

	// METHODS

	// helpers
	bool isDigitsOnly(const std::string &str) const;
	size_t strToSizeT(const std::string &str) const;
	std::string removeComments(const std::string &input) const;
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
	Request(const std::string &requestLineAndHeaders);
	~Request(){};

	// SETTERS

	void appendToBody(const std::vector<std::byte> &newBodyChunk);
	// Also add getters for all the info - or maybe a single getter?
	// no it's a stupid idea because I would need to define a struct for that...

	// GETTERS

	// request data getters
	// std::vector<std::byte> getBody() const;
	// size_t getContentLength() const;
	// status getters
	RequestStatus getStatus() const;
	// HttpStatusCode getStatusCode() const;
	bool bodyExpected() const;

	// EXCEPTIONS

	class BadRequestException : public std::exception
	{
	};
};

#endif
