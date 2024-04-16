#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <vector>
#include <queue>
#include <iostream>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

class Client
{

private:
	struct sockaddr_in address;
	socklen_t addrlen;
	Request *request;
	Response *response;
	size_t bytes_to_receive;

public:
	Client();
	~Client();

	struct sockaddr_in &getAndSetAddress(void);
	socklen_t &getAndSetAddrlen(void);

	void createRequest(std::string &request_header);
	void createResponse(void);

	void removeRequest(void);
	void removeResponse(void);

	Request *getRequest(void);
	Response *getResponse(void);
	size_t &getBytesToReceive(void);

	bool isNewRequest(void) const;

	void setBytesToReceive(size_t bytes);
};

#endif