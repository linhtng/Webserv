#include "Client.hpp"

Client::Client()
	: addrlen(sizeof(address)), request(NULL), response(NULL), chunk_size(0)
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
	request = new Request(request_header, config); // Create a Request object with the provided header
	chunk_size = 0;
}

void Client::createResponse()
{
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

size_t const &Client::getChunkSize() const
{
	return (chunk_size);
}

std::string const &Client::getRequestBodyBuf() const
{
	return (request_body_buf);
}

void Client::setChunkSize(size_t const &bytes)
{
	chunk_size = bytes;
}

void Client::setRequestBodyBuf(std::string const &buf)
{
	request_body_buf = buf;
}


