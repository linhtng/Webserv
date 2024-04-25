#ifndef REQUEST_HPP
#define REQUEST_HPP

// TODO: check includes
#include "../defines.hpp"
#include <string>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <limits>
#include "../HttpMessage/HttpMessage.hpp"

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
	size_t _chunkSize;
	std::vector<std::byte> _bodyBuf;

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
	Request(const ConfigData &config, const std::string &requestLineAndHeaders);
	Request(const ConfigData &config, HttpStatusCode statusCode);

	// SETTERS

	void appendToBody(const std::vector<std::byte> &newBodyChunk);
	void setChunkSize(const size_t &bytes);
	void setBodyBuf(const std::vector<std::byte> &buf);
	void clearBodyBuf();

	// GETTERS

	std::string getHost() const;
	std::string getUserAgent() const;
	bool isBodyExpected() const;
	std::string getTransferEncoding() const;
	size_t getChunkSize() const;
	std::vector<std::byte> getBodyBuf() const;

	// EXCEPTIONS

	class BadRequestException : public std::exception
	{
	};

	// DEBUG
	void printRequestProperties() const;
};

#endif
