#include "Client.hpp"

Client::Client() : addrlen(sizeof(address))
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
	return *this;
}

Client::~Client()
{
}

sockaddr_in &Client::getAndSetAddress(void)
{
	return address;
}

socklen_t &Client::getAndSetAddrlen(void)
{
	return addrlen;
}

std::string Client::getResponse(void) const
{
	return response;
}

void Client::setResponse(std::string new_response)
{
	response = new_response;
}
