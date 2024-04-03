#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../defines.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

class Request
{
private:
	struct RequestLine
	{
		std::string method;
		std::string requestTarget;
		std::string HTTPVersion;
		/* When a major version of HTTP does not define any minor versions, the minor version "0" is implied. A recipient that receives a message with a major version number that it implements and a minor version number higher than what it implements SHOULD process the message as if it were in the highest minor version within that major version to which the recipient is conformant. */
	};

	struct Header
	{
		std::string method;
		std::string requestTarget;
		std::string HTTPVersion;
	};

	RequestStatus _status;
	HttpStatusCode _statusCode;
	RequestLine _requestLine;
	/*
	The normal procedure for parsing an HTTP message is to read the start-line
	into a structure, read each header field line into a hash table by field
	name until the empty line, and then use the parsed data to determine if
	a message body is expected.
	 */
	std::unordered_map<std::string, std::string> _headerLines;
	std::vector<std::byte> _body;
	size_t _contentLength; // Since there is no predefined limit to the length of content, a recipient MUST anticipate potentially large decimal numerals and prevent parsing errors due to integer conversion overflows or precision loss due to integer conversion

	Request();

	void parseRequestLine(const std::string &requestLine);
	void parseHeaderLine(const std::string &headerLine);

public:
	// Constructor that takes a string representing request-line and header file lines
	Request(const std::string &requestLineAndHeaders);
	// Request(const Request &source);
	~Request(){};

	// Request &operator=(const Request &source);

	void setBody(const std::vector<std::byte> &str) const;
	// Also add getters for all the info - or maybe a single getter?
	// no it's a stupid idea because I would need to define a struct for that...

	std::vector<std::byte> getBody() const;
	RequestStatus getStatus() const;
	size_t getContentLength() const;
	bool bodyExpected() const;
};

// std::ostream &operator<<(std::ostream &out, const Request &request);

#endif
