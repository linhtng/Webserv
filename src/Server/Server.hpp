#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <algorithm>
#include <unistd.h>
#include <memory>
#include "Client.hpp"
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../config_parser/ConfigParser.hpp"
#include "../defines.hpp"
#include "../Logger/Logger.hpp"

class Server
{

public:
	enum RequestStatus
	{
		HEADER_DELIMITER_FOUND,
		BAD_HEADER,
		REQUEST_CLIENT_DISCONNECT,
		BODY_IN_CHUNK,
		READY_TO_WRITE,
		PARSED_CHUNK_SIZE,
		BAD_REQUEST,
		SERVER_ERROR,
		PAYLOAD_TOO_LARGE
	};

	enum ResponseStatus
	{
		RESPONSE_IN_CHUNK,
		RESPONSE_DISCONNECT_CLIENT,
		KEEP_ALIVE,
	};

private:
	int serverFd;
	std::vector<ConfigData> configs;
	std::unordered_map<int, std::unique_ptr<Client>> clients;
	struct sockaddr_in address;
	std::string host;
	int port;
	std::unordered_map<int, std::pair<const int *, const int *>> clientCgiPipeFds;

	RequestStatus receiveRequestHeader(int const &clientFd);
	RequestStatus formRequestHeader(int const &clientFd, std::string &requestHeader, std::vector<std::byte> &requestBodyBuf);
	RequestStatus processRequestHeaderBuf(int const &clientFd);
	RequestStatus receiveRequestBody(int const &clientFd);
	RequestStatus formRequestBodyWithContentLength(int const &clientFd, char readBuf[], ssize_t const &bytes);
	RequestStatus formRequestBodyWithChunk(int const &clientFd, char readBuf[], ssize_t const &bytes);
	RequestStatus processChunkData(int const &clientFd);
	RequestStatus extractChunkSize(int const &clientFd);
	Server();

public:
	Server(ConfigData &config);

	void setUpServerSocket();
	int acceptNewConnection();
	RequestStatus receiveRequest(int const &clientFd);
	ResponseStatus sendResponse(int const &clientFd);
	void createAndSendErrorResponse(HttpStatusCode const &statusCode, int const &clientFd);

	int const &getServerFd() const;
	std::string const &getHost();
	int const &getPort();
	unsigned short int const &getClientPortNumber(int const &clientFd);
	in_addr const &getClientIPv4Address(int const &clientFd);
	std::unordered_map<int, std::pair<const int *, const int *>> &getClientCgiPipeFds();

	void appendConfig(ConfigData const &config);
	void removeClient(int const &clientFd);

	class SocketCreationException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class SocketSetNonBlockingException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class SocketBindingException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class SocketListenException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class SocketSetOptionException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};
};

#endif
