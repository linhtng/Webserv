#ifndef SERVER_HPP
#define SERVER_HPP

#define BACKLOG 100
#define BUFFER_SIZE 1024

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
#include "Client.hpp"
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

class Server
{

public:
	typedef struct configData
	{
		int serverPort;
		std::string serverHost;
	} configData_t;

	enum RequestStatus
	{
		HEADER_DELIMITER_FOUND,
		HEADER_NO_DELIMITER,
		REQUEST_CLIENT_DISCONNECTED,
		BODY_IN_CHUNK,
		READY_TO_WRITE,
		REQUEST_INTERRUPTED
	};

	enum ResponseStatus
	{
		RESPONSE_CLIENT_DISCONNECTED,
		KEEP_ALIVE,
		CLOSE_CONNECTION,
		RESPONSE_INTERRUPTED
	};

private:
	int server_fd;
	configData_t config;
	std::unordered_map<int, Client> clients;
	struct sockaddr_in address;

	RequestStatus formRequestHeader(int const &client_fd, std::string &request_header, std::vector<std::byte> &body_message_buf);
	RequestStatus formRequestBody(int const &client_fd, std::vector<std::byte> &request_body_buf, Request &request);

public:
	Server();
	Server(configData_t &config);
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &rhs);

	void setUpServerSocket();
	std::vector<int> acceptNewConnections();
	RequestStatus receiveRequest(int const &client_fd);
	ResponseStatus sendResponse(int const &client_fd);

	int const &getServerFd(void) const;
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

	class TimeoutException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class RecvException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class SendException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};
};

#endif
