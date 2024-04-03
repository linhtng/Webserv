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

class Server
{

public:
	typedef struct configData
	{
		int serverPort;
		std::string serverHost;
	} configData_t;

private:
	int server_fd;
	configData_t config;
	std::unordered_map<int, Client> clients;

	Server();

public:
	enum ConnectionStatus
	{
		OPEN,
		CLOSE
	};

	Server(configData_t &config);
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &rhs);

	void setUpServerSocket();
	std::vector<int> acceptNewConnections();
	ConnectionStatus receiveRequest(int const &client_fd);
	ssize_t formRequestHeader(int const &client_fd, std::string &request_header, std::string &body_message_buf);
	ConnectionStatus sendResponse(int const &client_fd);

	bool isClient(int const &client_fd) const;

	int const &getServerFd(void) const;

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
