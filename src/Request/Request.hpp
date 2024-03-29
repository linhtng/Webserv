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
		std::string HTTPVersions;
	};

	bool _error;
	RequestStatus _status;
	bool _bodyExpected;
	RequestLine _requestLine;
	/*
	The normal procedure for parsing an HTTP message is to read the start-line
	into a structure, read each header field line into a hash table by field
	name until the empty line, and then use the parsed data to determine if
	a message body is expected.
	 */
	std::unordered_map<std::string, std::string> _headerLines;
	std::vector<std::byte> _body;

	Request();

public:
	// Constructor that takes a string representing request-line and header file lines
	Request(const std::string &requestLine);
	// Request(const Request &source);
	~Request();

	// Request &operator=(const Request &source);

	void setBody(const std::vector<std::byte> &str) const;
	// Also add getters for all the info - or maybe a single getter?
	// no it's a stupid idea because I would need to define a struct for that...

	std::vector<std::byte> getBody() const;
	bool isError() const;
	RequestStatus getStatus const;
	bool bodyExpected() const;
};

std::ostream &operator<<(std::ostream &out, const Request &request);

#endif
