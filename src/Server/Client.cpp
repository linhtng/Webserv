#include "Client.hpp"

Client::Client()
	: addrlen(sizeof(address)), request(NULL), response(NULL), bytes_to_receive(0)
{
}

Client::Client(Client const &src)
{
	*this = src;
}

Client &Client::operator=(Client const &rhs)
{
	if (this != &rhs)
	{
		address = rhs.address;
		addrlen = rhs.addrlen;
	}
	return (*this);
}

Client::~Client()
{
	delete request;
	delete response;
}

sockaddr_in &Client::getAndSetAddress(void)
{
	return (address);
}

socklen_t &Client::getAndSetAddrlen(void)
{
	return (addrlen);
}

void Client::createRequest(std::string &request_header)
{
	request = new Request(request_header); // Create a Request object with the provided header
}

void Client::createResponse(void)
{
	response = new Response(*request); // Create a Response object with the corresponding request header
}

void Client::removeRequest(void)
{
	delete request;
	request = NULL;
}

void Client::removeResponse(void)
{
	delete response;
	response = NULL;
}

Request *Client::getRequest(void)
{
	return request;
}

Response *Client::getResponse(void)
{
	return response;
}

int &Client::getBytesToReceive(void)
{
	return bytes_to_receive;
}

void Client::setBytesToReceive(int bytes)
{
	bytes_to_receive = bytes;
}

void Client::addChunkSize(size_t size)
{
	chunk_sizes.push_back(size);
}
