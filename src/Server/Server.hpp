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

class Server
{

private:
	int server_fd;
	std::unordered_map<std::string, std::string> config;
	std::vector<int> client_fds;

	Server();

	struct
	{
		struct sockaddr_in address;
		socklen_t addrlen;
	} client;

public:
	enum ConnectionStatus
	{
		OPEN,
		CLOSE
	};

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

	Server(const std::unordered_map<std::string, std::string> &config);
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &rhs);

	void setUpServerSocket();
	std::vector<int> acceptNewConnection();
	ConnectionStatus receiveRequest(int const &client_fd);
	ConnectionStatus sendResponse(int const &client_fd);

	int const &getServerFd(void) const;
	std::vector<int> const &getClientFds(void) const;
};

#endif
