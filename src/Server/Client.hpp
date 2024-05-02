#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <iostream>
#include <string>
#include <memory>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../config_parser/ConfigData.hpp"

class Client
{

private:
	struct sockaddr_in address;
	socklen_t addrlen;
	std::unique_ptr<Request> request;
	std::unique_ptr<Response> response;
	bool is_connection_close;
	Client();

	// Helper properties for sending
	size_t _bytesSent;

	// Helper properties for parsing
	size_t _chunkSize;
	std::vector<std::byte> _bodyBuf;
	size_t _bytesToReceive;

public:
	Client(struct sockaddr_in client_address, socklen_t client_addrlen);
	~Client();

	void createRequest(std::string const &request_header, ConfigData const &config);
	void createResponse();

	void createErrorRequest(ConfigData const &config, HttpStatusCode statusCode);

	void removeRequest();
	void removeResponse();

	bool isNewRequest() const;

	const Request &getRequest() const;
	const Response &getResponse() const;
	bool const &getIsConnectionClose() const;

	unsigned short int const &getPortNumber() const;
	struct in_addr const &getIPv4Address() const;

	void setIsConnectionClose(bool const &status);

	void setBytesSent(size_t const &bytes);
	size_t const &getBytesSent() const;

	void appendToBodyBuf(const std::vector<std::byte> &buf);
	void appendToBodyBuf(char buf[], const ssize_t &bytes);
	void eraseBodyBuf(const size_t &start, const size_t &end);
	void clearBodyBuf();
	void setChunkSize(const size_t &bytes);
	void setBytesToReceive(size_t bytes);

	size_t getChunkSize() const;
	std::vector<std::byte> getBodyBuf() const;
	size_t getBytesToReceive() const;

	void appendToRequestBody(const std::vector<std::byte> &newBodyChunk);
	void appendToRequestBody(char newBodyChunk[], const ssize_t &bytes);
	void resizeRequestBody(const size_t &n);

	std::vector<std::byte> getRequestBody() const;
	HttpMethod getRequestMethod() const;
	std::string getRequestTarget() const;

	bool isRequestBodyExpected() const;
	bool isRequestChunked() const;
	HttpStatusCode getRequestStatusCode() const;

	size_t getRequestContentLength() const;

	ConnectionValue getConnection() const;
};

#endif