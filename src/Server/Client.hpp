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

		size_t _bytesSend;


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
};

#endif