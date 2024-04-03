#include "Server.hpp"
#include <cstring>

Server::Server(configData_t &config) : server_fd(0), config(config)
{
}

Server::Server(Server const &src)
{
	*this = src;
}

Server &Server::operator=(Server const &rhs)
{
	if (this != &rhs)
	{
		server_fd = rhs.server_fd;
		config = rhs.config;
	}

	return *this;
}

Server::~Server()
{
}

// set up server socket
void Server::setUpServerSocket()
{
	struct sockaddr_in address;
	int opt = 1;

	try
	{
		server_fd = socket(AF_INET, SOCK_STREAM, 0); // create socket file descriptor
		if (server_fd < 0)
			throw SocketCreationException();

		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, // set file descriptor to be reuseable
					   sizeof(opt)) < 0)
			throw SocketSetOptionException();

		if (fcntl(server_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
			throw SocketSetNonBlockingException();

		address.sin_family = AF_INET;
		address.sin_port = htons(config.serverPort);
		address.sin_addr.s_addr = inet_addr(config.serverHost.c_str());

		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) // bind the socket to the address and port number
			throw SocketBindingException();

		if (listen(server_fd, BACKLOG) < 0) // set server socket in passive mode
			throw SocketListenException();
	}
	catch (std::exception &e)
	{
		if (server_fd >= 0)
			close(server_fd);
		throw;
	}
}

// accept new connections
std::vector<int> Server::acceptNewConnections()
{
	std::vector<int> client_fds;
	while (true)
	{
		Client client;
		int client_fd = accept(server_fd,
							   (struct sockaddr *)&(client.getAndSetAddress()),
							   &(client.getAndSetAddrlen()));
		if (client_fd < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) // listen() queue is empty or interrupted by a signal
				break;
			else if (errno == ECONNABORTED) // a connection has been aborted
				continue;
			throw AcceptException();
		}
		if (fcntl(client_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
		{
			close(client_fd);
			for (const int client_fd : client_fds) // close all pollfds
				close(client_fd);
			throw SocketSetNonBlockingException();
		}
		clients[client_fd] = client;
		client_fds.push_back(client_fd);
	}

	return client_fds;
}

// receive the request
Server::ConnectionStatus Server::receiveRequest(int const &client_fd)
{
	std::string request_header;
	std::string body_message_buf;

	ssize_t bytes = formRequestHeader(client_fd, request_header, body_message_buf);

	if (bytes == 0)
		return ConnectionStatus::CLOSE;
	else if (bytes < 0)
		return ConnectionStatus::OPEN;

	std::cout << "body message buf: " << body_message_buf << std::endl;
	std::cout << "request_header: " << request_header << std::endl;

	// Request request(headerStr);
	// if (request.success)
	// {
	// body = body_message_buf + loop for read
	// read body for request.bytes
	//	request.addBody(bodyStr);
	// }
	// Response (request);
	// save response to client

	clients[client_fd].setResponse("Hello from server");
	return ConnectionStatus::OPEN;
}

ssize_t Server::formRequestHeader(int const &client_fd, std::string &request_header, std::string &body_message_buf)
{
	char buf[BUFFER_SIZE];
	std::string delimitor = "\r\n\r\n";
	size_t delimitor_pos;
	ssize_t bytes;

	do
	{
		bytes = recv(client_fd, buf, sizeof(buf), 0);
		if (bytes == 0) // connection has been closed by the client, close connection, remove fd and remove client
		{
			clients.erase(client_fd);
			return bytes;
		}

		if (bytes < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // if no messages are available, send error to client
			{
				clients[client_fd].setResponse("timeout"); // TODO - replace with response to client
				perror("timeout");
				return bytes;
			}
			else if (errno == EINTR)
				return bytes;
			throw RecvException();
		}

		request_header.append(buf, bytes);
		delimitor_pos = request_header.find(delimitor);
	} while (delimitor_pos == std::string::npos); // read until \r\n\r\n

	body_message_buf = request_header.substr(delimitor_pos + delimitor.length()); // store the message body that is already read into buf
	request_header.erase(delimitor_pos);

	return bytes;
}

// send the response
Server::ConnectionStatus Server::sendResponse(int const &client_fd)
{
	std::string response = clients[client_fd].getResponse();

	// send response
	send(client_fd, response.c_str(), response.length(), 0);

	std::cout << "Response sent from server" << std::endl;

	// keep the connection by default
	// return ConnectionStatus::OPEN;
	// if request header = close, close connection, remove fd and remove client
	clients.erase(client_fd);
	return ConnectionStatus::CLOSE;
}

bool Server::isClient(int const &client_fd) const
{
	if (clients.find(client_fd) == clients.end())
		return false;
	return true;
}

int const &Server::getServerFd(void) const
{
	return server_fd;
}

const char *Server::SocketCreationException::what() const throw()
{
	return ("Server::Fail to create socket");
}

const char *Server::SocketBindingException::what() const throw()
{
	return ("Server::Fail to bind socket");
}

const char *Server::SocketListenException::what() const throw()
{
	return ("Server::Fail to set socket in passive mode");
}

const char *Server::SocketSetNonBlockingException::what() const throw()
{
	return ("Server::Fail to set socket as non-blocking");
}

const char *Server::SocketSetOptionException::what() const throw()
{
	return ("Server::Fail to set socket options");
}

const char *Server::AcceptException::what() const throw()
{
	return ("Server::accept() failed");
}

const char *Server::TimeoutException::what() const throw()
{
	return ("Server::Operation timeout");
}

const char *Server::RecvException::what() const throw()
{
	return ("Server::recv() failed");
}

const char *Server::SendException::what() const throw()
{
	return ("Server::send() failed");
}
