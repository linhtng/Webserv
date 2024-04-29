#include "Client.hpp"

Client::Client()
	: addrlen(sizeof(address)), request(NULL), response(NULL)
{
}

Client::~Client()
{
	delete request;
	delete response;
}

sockaddr_in &Client::getAndSetAddress()
{
	return (address);
}

socklen_t &Client::getAndSetAddrlen()
{
	return (addrlen);
}

void Client::createRequest(std::string const &request_header, ConfigData const &config)
{
	removeRequest();
	request = new Request(config, request_header); // Create a Request object with the provided header
	request->setChunkSize(0);
}

void Client::createErrorRequest(ConfigData const &config, HttpStatusCode statusCode)
{
	removeRequest();
	request = new Request(config, statusCode); // Create a Request object with the provided header
}

void Client::createResponse()
{
	removeResponse();
	response = new Response(*request); // Create a Response object with the corresponding request
}

void Client::removeRequest()
{
	if (request)
	{
		delete request;
		request = NULL;
	}
}

void Client::removeResponse()
{
	if (response)
	{
		delete response;
		response = NULL;
	}
}

bool Client::isNewRequest() const
{
	return request ? false : true;
}

Request *Client::getRequest() const
{
	return (request);
}

Response *Client::getResponse() const
{
	return (response);
}

unsigned short int const &Client::getPortNumber() const
{
	return (address.sin_port);
}

in_addr const &Client::getIPv4Address() const
{
	return (address.sin_addr);
}
