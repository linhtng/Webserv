#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <iostream>
#include <string>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../config_parser/ConfigData.hpp"

class Client
{

private:
	struct sockaddr_in address;
	socklen_t addrlen;
	Request *request;
	Response *response;
	std::string request_header_buf;

public:
	Client();
	~Client();

	struct sockaddr_in &getAndSetAddress();
	socklen_t &getAndSetAddrlen();

	void createRequest(std::string const &request_header, ConfigData const &config);
	void createResponse();

	void createErrorRequest(ConfigData const &config, HttpStatusCode statusCode);

	void removeRequest();
	void removeResponse();

	bool isNewRequest() const;

	Request *getRequest() const;
	Response *getResponse() const;
	std::string const &getRequestHeaderBuf() const;

	unsigned short int const &getPortNumber() const;
	struct in_addr const &getIPv4Address() const;

	void appendToRequestHeaderBuf(char buf[], const ssize_t &bytes);
	void resizeRequestHeaderBuf(const size_t &size);
	void clearRequestHeaderBuf();
};

#endif