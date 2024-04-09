#include "Client.hpp"

Client::Client()
	: addrlen(sizeof(address))
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
		response = rhs.response;
	}
	return (*this);
}

Client::~Client()
{
}

sockaddr_in &Client::getAndSetAddress(void)
{
	return (address);
}

socklen_t &Client::getAndSetAddrlen(void)
{
	return (addrlen);
}

std::vector<std::byte> Client::getResponse(void) const
{
	return (response);
}

void Client::setRequest(std::vector<std::byte> new_request_chunk)
{
	request.insert(request.end(), new_request_chunk.begin(), new_request_chunk.end());
}

void Client::setResponse(std::vector<std::byte> new_response)
{
	response = new_response;
}
