#include "Client.hpp"

Client::Client() : client_fd(0), addrlen(sizeof(address))
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
		client_fd = rhs.client_fd;
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

const int &Client::getClientFd(void) const
{
	return client_fd;
}

std::string Client::getResponse(void) const
{
	return response;
}

void Client::setFd(int new_fd)
{
	client_fd = new_fd;
}

void Client::setResponse(std::string new_response)
{
	response = new_response;
}
