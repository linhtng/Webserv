#include "Client.hpp"

Client::Client(sockaddr_in client_address, socklen_t client_addrlen)
		: address(client_address), addrlen(client_addrlen), request(nullptr), response(nullptr), is_connection_close(false)
{
	std::cout << YELLOW << "constructor of client is called" << RESET << std::endl;
}

Client::~Client()
{
	std::cout << YELLOW << "destrcutor of client is called" << RESET << std::endl;
}

void Client::createRequest(std::string const &request_header, ConfigData const &config)
{
	removeRequest();
	request = std::make_unique<Request>(config, request_header); // Create a Request object with the provided header
	request->setChunkSize(0);
}

void Client::createErrorRequest(ConfigData const &config, HttpStatusCode statusCode)
{
	removeRequest();
	request = std::make_unique<Request>(config, statusCode); // Create a Request object with the provided header
}

void Client::createResponse()
{
	removeResponse();
	response = std::make_unique<Response>(*request); // Create a Response object with the corresponding request
}

void Client::removeRequest()
{
	request.reset();
}

void Client::removeResponse()
{
	response.reset();
}

bool Client::isNewRequest() const
{
	return request ? false : true;
}

const Request &Client::getRequest() const {
    return *request;
}

const Response &Client::getResponse() const {
    return *response;
}

bool const &Client::getIsConnectionClose() const
{
	return (is_connection_close);
}

unsigned short int const &Client::getPortNumber() const
{
	return (address.sin_port);
}

in_addr const &Client::getIPv4Address() const
{
	return (address.sin_addr);
}

void Client::setIsConnectionClose(bool const &status)
{
	is_connection_close = status;
}
