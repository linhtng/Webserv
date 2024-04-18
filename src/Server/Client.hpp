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
	std::vector<size_t> chunk_sizes;

public:
	Client();
	Client(Client const &src);
	Client &operator=(Client const &rhs);
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

	void setBytesToReceive(size_t bytes);
	void addChunkSize(size_t size);
};

#endif