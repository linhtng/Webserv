#include "Client.hpp"

Client::Client(sockaddr_in client_address, socklen_t client_addrlen)
	: address(client_address), addrlen(client_addrlen), request(nullptr), response(nullptr), is_connection_close(false), _bytesSent(0), _chunkSize(0),
	  _bytesToReceive(0)
{
	std::cout << YELLOW << "constructor of client is called" << RESET << std::endl;
}

Client::~Client()
{
	std::cout << YELLOW << "destrcutor of client is called" << RESET << std::endl;
}

void Client::setBytesSent(size_t const &bytes)
{
	_bytesSent = bytes;
}
size_t const &Client::getBytesSent() const
{
	return (_bytesSent);
}

void Client::createRequest(std::string const &request_header, std::vector<ConfigData> const &configs)
{
	removeRequest();
	request = std::make_unique<Request>(configs, request_header); // Create a Request object with the provided header
	setChunkSize(0);
}

void Client::createErrorRequest(std::vector<ConfigData> const &configs, HttpStatusCode statusCode)
{
	removeRequest();
	request = std::make_unique<Request>(configs, statusCode); // Create a Request object with the provided header
}

void Client::createResponse()
{
	removeResponse();
	response = std::make_unique<Response>(*request); // Create a Response object with the corresponding request
	setBytesSent(0);
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

const Request &Client::getRequest() const
{
	return *request;
}

const Response &Client::getResponse() const
{
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

void Client::appendToBodyBuf(const std::vector<std::byte> &buf)
{
	this->_bodyBuf.insert(this->_bodyBuf.end(), buf.begin(), buf.end());
}

void Client::appendToBodyBuf(char buf[], const ssize_t &bytes)
{
	for (ssize_t i = 0; i < bytes; ++i)
		this->_bodyBuf.push_back(static_cast<std::byte>(buf[i]));
}

void Client::eraseBodyBuf(const size_t &start, const size_t &end)
{
	this->_bodyBuf.erase(_bodyBuf.begin() + start, _bodyBuf.begin() + end);
}

void Client::clearBodyBuf()
{
	this->_bodyBuf.clear();
}

void Client::setChunkSize(const size_t &bytes)
{
	_chunkSize = bytes;
}

void Client::setBytesToReceive(size_t bytes)
{
	this->_bytesToReceive = bytes;
}

size_t Client::getChunkSize() const
{
	return this->_chunkSize;
}

std::vector<std::byte> Client::getBodyBuf() const
{
	return this->_bodyBuf;
}

size_t Client::getBytesToReceive() const
{
	return this->_bytesToReceive;
}

void Client::appendToRequestBody(const std::vector<std::byte> &newBodyChunk)
{
	if (request)
		request->appendToBody(newBodyChunk);
}

void Client::appendToRequestBody(char newBodyChunk[], const ssize_t &bytes)
{
	if (request)
		request->appendToBody(newBodyChunk, bytes);
}

void Client::resizeRequestBody(const size_t &n)
{
	if (request)
		request->resizeBody(n);
}

std::vector<std::byte> Client::getRequestBody() const
{
	return (request->getBody());
}
