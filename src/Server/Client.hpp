#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <vector>
#include <queue>
#include <iostream>
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
	// size_t chunk_size; //TODO - move to request
	// std::string request_body_buf; //TODO - move to request

public:
	Client();
	~Client();

	struct sockaddr_in &getAndSetAddress();
	socklen_t &getAndSetAddrlen();

	void createRequest(std::string const &request_header, ConfigData const &config);
	void createResponse();

	void removeRequest();
	void removeResponse();

	bool isNewRequest() const;

	Request *getRequest() const;
	Response *getResponse() const;
	// size_t const &getChunkSize() const;
	// std::string const &getRequestBodyBuf() const;

	// void setChunkSize(size_t const &bytes);
	// void setRequestBodyBuf(std::string const &buf);
};

#endif