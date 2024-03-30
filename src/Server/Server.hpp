#ifndef SERVER_HPP
#define SERVER_HPP

#define BACKLOG 100
#define BUFFER_SIZE 1024

#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <list>

typedef typename std::vector<std::unordered_map<std::string, std::string>> config_t;
typedef typename std::unordered_map<int, std::unordered_map<std::string, std::string>> server_sockets_t;

class Server
{

private:
	Server(Server const &src);
	Server &operator=(Server const &rhs);

	server_sockets_t server_sockets;
	std::list<pollfd> fds;

	void setUpServerSocket();
	void serverLoop();
	void acceptNewConnection(const int &server_fd);
	void receiveRequest(std::list<pollfd>::iterator &it);
	void sendResponse(std::list<pollfd>::iterator &it);

	// to be replaced by config file
	config_t servers;

	struct
	{
		struct sockaddr_in address;
		socklen_t addrlen;
	} client;

public:
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

	class PollException : public std::exception
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

	class PollErrorException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	Server();
	~Server();

	void runServer();
};

#endif
