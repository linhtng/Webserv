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
	bool isConnectionClose;

	std::unique_ptr<Request> request;
	size_t chunkSize;
	std::vector<std::byte> bodyBuf;
	size_t bytesToReceive;

	std::unique_ptr<Response> response;
	size_t bytesSent;

	Client();

public:
	Client(struct sockaddr_in clientAddress);
	~Client();

	void createRequest(std::string const &requestHeader, std::vector<ConfigData> const &configs);
	void createErrorRequest(std::vector<ConfigData> const &configs, HttpStatusCode statusCode);
	void createResponse();

	void removeRequest();
	void removeResponse();

	const Request &getRequest() const;
	const Response &getResponse() const;

	bool isNewRequest() const;

	bool const &getIsConnectionClose() const;
	unsigned short int const &getPortNumber() const;
	struct in_addr const &getIPv4Address() const;
	size_t const &getBytesSent() const;
	size_t getChunkSize() const;
	size_t getBytesToReceive() const;
	std::vector<std::byte> getBodyBuf() const;

	void setIsConnectionClose(bool const &status);
	void setBytesSent(size_t const &bytes);
	void setChunkSize(const size_t &bytes);
	void setBytesToReceive(size_t bytes);

	void appendToBodyBuf(const std::vector<std::byte> &buf);
	void appendToBodyBuf(char buf[], const ssize_t &bytes);
	void eraseBodyBuf(const size_t &start, const size_t &end);
	void clearBodyBuf();

	void appendToRequestBody(const std::vector<std::byte> &newBodyChunk);
	void appendToRequestBody(char newBodyChunk[], const ssize_t &bytes);
	void resizeRequestBody(const size_t &n);
	std::vector<std::byte> getRequestBody() const;
};

#endif