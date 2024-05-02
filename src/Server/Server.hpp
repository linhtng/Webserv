#ifndef SERVER_HPP
#define SERVER_HPP

#define BACKLOG 100
#define BUFFER_SIZE 200000
#define MAX_HEADER_LENGTH 8192

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
#include <regex>
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
		REQUEST_DISCONNECT_CLIENT,
		BODY_IN_CHUNK,
		READY_TO_WRITE,
		PARSED_CHUNK_SIZE,
		BAD_REQUEST,
		BODY_EXPECTED,
		SERVER_ERROR
	};

	enum ResponseStatus
	{
		RESPONSE_IN_CHUNK,
		RESPONSE_DISCONNECT_CLIENT,
		KEEP_ALIVE,
	};

private:
	int server_fd;
	ConfigData config;
	std::unordered_map<int, std::unique_ptr<Client>> clients;
	struct sockaddr_in address;

	RequestStatus formRequestHeader(int const &client_fd, std::string &request_header, std::vector<std::byte> &request_body_buf);
	RequestStatus formRequestBodyWithContentLength(int const &client_fd);
	RequestStatus formRequestBodyWithChunk(int const &client_fd);
	RequestStatus processChunkData(int const &client_fd);
	RequestStatus extractChunkSize(int const &client_fd);
	Server();

public:
	Server(ConfigData &config);
	~Server();

	void setUpServerSocket();
	std::vector<int> acceptNewConnections();
	RequestStatus receiveRequest(int const &client_fd);
	ResponseStatus sendResponse(int const &client_fd);
	void createAndSendErrorResponse(HttpStatusCode const &statusCode, int const &client_fd);

	int const &getServerFd() const;
	ConfigData const &getConfig() const;
	unsigned short int const &getClientPortNumber(int const &client_fd);
	in_addr const &getClientIPv4Address(int const &client_fd);

	void removeClient(int const &client_fd);

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

	class AcceptException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};
};

#endif
