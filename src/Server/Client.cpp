#include "Client.hpp"

Client::Client(sockaddr_in clientAddress)
	: address(clientAddress), request(nullptr), response(nullptr), isConnectionClose(false), bytesSent(0), chunkSize(0),
	  bytesToReceive(0)
{
}

void Client::setBytesSent(size_t const &bytes)
{
	bytesSent = bytes;
}
size_t const &Client::getBytesSent() const
{
	return (bytesSent);
}

void Client::createRequest(std::string const &requestHeader, std::vector<ConfigData> const &configs)
{
	removeRequest();
	request = std::make_unique<Request>(configs, requestHeader); // Create a Request object with the provided header
	bytesSent = 0;
	chunkSize = 0;
	bytesToReceive = 0;
	bodyBuf.clear();
}

void Client::createErrorRequest(std::vector<ConfigData> const &configs, HttpStatusCode statusCode)
{
	removeRequest();
	Logger::log(ERROR, SERVER, "Creating error request with status code: %s ", statusCode);
	request = std::make_unique<Request>(configs, statusCode); // Create a Request object with the provided header
}

void Client::createResponse()
{
	removeResponse();
	response = std::make_unique<Response>(*request); // Create a Response object with the corresponding request
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

bool Client::isNewRequest() const
{
	return (request ? false : true);
}

const Request &Client::getRequest() const
{
	return (*request);
}

const Response &Client::getResponse() const
{
	return (*response);
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

void Client::setIsConnectionClose(bool const &status)
{
	isConnectionClose = status;
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

void Client::setChunkSize(const size_t &bytes)
{
	chunkSize = bytes;
}

void Client::setBytesToReceive(size_t bytes)
{
	bytesToReceive = bytes;
}

size_t Client::getChunkSize() const
{
	return (chunkSize);
}

std::vector<std::byte> Client::getBodyBuf() const
{
	return (bodyBuf);
}

size_t Client::getBytesToReceive() const
{
	return (bytesToReceive);
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
