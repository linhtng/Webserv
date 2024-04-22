#include "Client.hpp"

Client::Client()
	: addrlen(sizeof(address)), request(NULL), response(NULL), bytes_to_receive(0)
{
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

void Client::createRequest(std::string const &request_header, ConfigData const &config)
{
	request = new Request(request_header, config); // Create a Request object with the provided header
	bytes_to_receive = 0;
}

void Client::createResponse(void)
{
	response = new Response(*request); // Create a Response object with the corresponding request
}

void Client::removeRequest(void)
{
	if (request)
	{
		delete request;
		request = NULL;
	}
}

void Client::removeResponse(void)
{
	if (response)
	{
		delete response;
		response = NULL;
	}
}

Request *Client::getRequest(void)
{
	return request;
}

Response *Client::getResponse(void)
{
	return response;
}

size_t &Client::getBytesToReceive(void)
{
	return bytes_to_receive;
}

bool Client::isNewRequest(void) const
{
	if (!request)
		return (true);
	return (false);
}

void Client::setBytesToReceive(size_t bytes)
{
	bytes_to_receive = bytes;
}
