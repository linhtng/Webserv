#include "Client.hpp"

Client::Client(sockaddr_in clientAddress)
	: address(clientAddress), isConnectionClose(false), request(nullptr),
	  chunkSize(0), bytesToReceive(0), response(nullptr), bytesSent(0)
{
	std::cout << YELLOW << "constructor of client is called" << RESET << std::endl;
}

Client::~Client()
{
	std::cout << YELLOW << "destrcutor of client is called" << RESET << std::endl;
}

void Client::createRequest(std::string const &requestHeader, std::vector<ConfigData> const &configs)
{
	removeRequest();
	request = std::make_unique<Request>(configs, requestHeader);
	chunkSize = 0;
	bytesToReceive = 0;
	bodyBuf.clear();
}

void Client::createErrorRequest(std::vector<ConfigData> const &configs, HttpStatusCode statusCode)
{
	removeRequest();
	request = std::make_unique<Request>(configs, statusCode);
	chunkSize = 0;
	bytesToReceive = 0;
	bodyBuf.clear();
}

void Client::createResponse()
{
	removeResponse();
	response = std::make_unique<Response>(*request);
	bytesSent = 0;
}

void Client::removeRequest()
{
	request.reset();
}

void Client::removeResponse()
{
	response.reset();
}

const Request &Client::getRequest() const
{
	return (*request);
}

const Response &Client::getResponse() const
{
	return (*response);
}

bool Client::isNewRequest() const
{
	return (request ? false : true);
}

bool const &Client::getIsConnectionClose() const
{
	return (isConnectionClose);
}

unsigned short int const &Client::getPortNumber() const
{
	return (address.sin_port);
}

in_addr const &Client::getIPv4Address() const
{
	return (address.sin_addr);
}

size_t const &Client::getBytesSent() const
{
	return (bytesSent);
}

size_t Client::getChunkSize() const
{
	return (chunkSize);
}

size_t Client::getBytesToReceive() const
{
	return (bytesToReceive);
}

std::vector<std::byte> Client::getBodyBuf() const
{
	return (bodyBuf);
}

void Client::setIsConnectionClose(bool const &status)
{
	isConnectionClose = status;
}

void Client::setBytesSent(size_t const &bytes)
{
	bytesSent = bytes;
}

void Client::setChunkSize(const size_t &bytes)
{
	chunkSize = bytes;
}

void Client::setBytesToReceive(size_t bytes)
{
	bytesToReceive = bytes;
}

void Client::appendToBodyBuf(const std::vector<std::byte> &buf)
{
	bodyBuf.insert(bodyBuf.end(), buf.begin(), buf.end());
}

void Client::appendToBodyBuf(char buf[], const ssize_t &bytes)
{
	for (ssize_t i = 0; i < bytes; ++i)
		bodyBuf.push_back(static_cast<std::byte>(buf[i]));
}

void Client::eraseBodyBuf(const size_t &start, const size_t &end)
{
	bodyBuf.erase(bodyBuf.begin() + start, bodyBuf.begin() + end);
}

void Client::clearBodyBuf()
{
	bodyBuf.clear();
}

void Client::appendToRequestBody(const std::vector<std::byte> &newBodyChunk)
{
	request->appendToBody(newBodyChunk);
}

void Client::appendToRequestBody(char newBodyChunk[], const ssize_t &bytes)
{
	request->appendToBody(newBodyChunk, bytes);
}

void Client::resizeRequestBody(const size_t &n)
{
	request->resizeBody(n);
}

std::vector<std::byte> Client::getRequestBody() const
{
	return (request->getBody());
}
