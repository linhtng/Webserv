#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../defines.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <algorithm>

class Request
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

	// PROPERTIES

	// for internal use only
	std::unordered_map<std::string, std::string> _headerLines;
	RequestLine _requestLine;

	RequestStatus _status;
	HttpStatusCode _statusCode;
	HttpMethod _method;
	std::string _requestTarget;
	int _HTTPVersionMajor;
	bool chunked;
	size_t _contentLength;
	std::vector<std::byte> _body;
	/*
	The normal procedure for parsing an HTTP message is to read the start-line
	into a structure, read each header field line into a hash table by field
	name until the empty line, and then use the parsed data to determine if
	a message body is expected.
	*/

	RequestLine _requestLine;

	// CONSTRUCTORS

	Request();

	// METHODS

	bool isDigitsOnly(const std::string &str) const;

	void extractRequestLine(const std::string &requestLine);
	void extractHeaderLine(const std::string &headerLine);
	void parseRequestLine();
	void parseHost();
	void parseContentLength();
	void parseHeaders();
	void processRequest(const std::string &requestLineAndHeaders);

public:
	Request(const std::string &requestLineAndHeaders);
	~Request(){};

	// SETTERS

	void appendToBody(const std::vector<std::byte> &newBodyChunk);
	// Also add getters for all the info - or maybe a single getter?
	// no it's a stupid idea because I would need to define a struct for that...

	// GETTERS
	std::vector<std::byte> getBody() const;
	RequestStatus getStatus() const;
	size_t getContentLength() const;
	HttpStatusCode getStatusCode() const;
	bool bodyExpected() const;

	class BadRequestException : public std::exception
	{
		virtual const char *what() const throw()
		{
			return "Bad Request";
		}
	};
};

#endif
