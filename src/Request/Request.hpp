#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Request
{
private:
	struct RequestLine
	{
		std::vector<std::byte> method;
		std::vector<std::byte> requestTarget;
		std::vector<std::byte> HTTPVersions;
	};
	bool _error;
	bool _bodyExpected;
	RequestLine _requestLine;
	/*
	The normal procedure for parsing an HTTP message is to read the start-line
	into a structure, read each header field line into a hash table by field
	name until the empty line, and then use the parsed data to determine if
	a message body is expected.
	 */
	std::unordered_map<std::vector<std::byte>, std::vector<std::byte>> _headerLines;
	std::vector<std::byte> _body;

public:
	// Constructor that takes a string representing request-line and header file lines
	Request(const std::vector<std::byte> &str);
	~Request();

	void setBody(const std::vector<std::byte> &str) const;
	// Also add getters for all the info - or maybe a single getter?
	// no it's a stupid idea because I would need to define a struct for that...

	std::vector<std::byte> getBody() const;
	bool isError() const;
	bool bodyExpected() const;
};

#endif
