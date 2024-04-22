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
#include <regex>
#include "Client.hpp"
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../config_parser/ConfigParser.hpp"
#include "../defines.hpp"

class Server
{

public:
	enum RequestStatus
	{
		HEADER_DELIMITER_FOUND,
		REQUEST_CLIENT_DISCONNECTED,
		BODY_IN_PART,
		BODY_IN_CHUNK,
		READY_TO_WRITE,
		REQUEST_INTERRUPTED,
		PARSED_CHUNK_BYTE,
		BAD_REQUEST,
		BODY_EXPECTED,
		NO_REQUEST_BODY
	};

	enum ResponseStatus
	{
		RESPONSE_CLIENT_DISCONNECTED,
		KEEP_ALIVE,
		RESPONSE_INTERRUPTED
	};

private:
	int server_fd;
	ConfigData config;
	std::unordered_map<int, Client> clients;
	struct sockaddr_in address;

	RequestStatus createRequestWithHeader(int const &client_fd, std::string &request_body_buf);
	RequestStatus formRequestHeader(int const &client_fd, std::string &request_header, std::string &body_message_buf);
	RequestStatus formRequestBodyWithContentLength(int const &client_fd, Request &request);
	RequestStatus formRequestBodyWithChunk(int const &client_fd, Request &request, std::string &request_body_buf);
	RequestStatus formRequestBodyWithChunkLoop(int const &client_fd, Request &request, std::string &body_buf, std::string &body);
	RequestStatus extractByteNumberFromChunk(std::string &str, int const &client_fd);
	void appendToBodyString(std::string &str, Request &request);					// TODO - move to request class
	void appendToBodyString(char buf[BUFFER_SIZE], size_t bytes, Request &request); // TODO - move to request class

public:
	Server();
	Server(ConfigData &config);

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
