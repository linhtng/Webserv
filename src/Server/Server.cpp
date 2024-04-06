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
		clients = rhs.clients;
		address = rhs.address;
	}
	return *this;
}

Server::~Server()
{
}

// set up server socket
void Server::setUpServerSocket()
{
	int opt = 1;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create socket file descriptor
		throw SocketCreationException();
	try
	{
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
		close(server_fd);
		throw;
	}
}

// accept new connections
std::vector<int> Server::acceptNewConnections()
{
	std::vector<int> client_fds;
	int client_fd;

	try
	{
		while (true)
		{
			Client client;
			client_fd = accept(server_fd,
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
				throw SocketSetNonBlockingException();
			clients[client_fd] = client;
			client_fds.push_back(client_fd);
		}
		return client_fds;
	}
	catch (std::exception &e)
	{
		if (client_fd >= 0)
			close(client_fd);
		for (const int fd : client_fds) // close all client fds
			close(fd);
		throw;
	}
}

// receive the request
Server::ConnectionStatus Server::receiveRequest(int const &client_fd)
{
	std::string request_header;
	std::vector<std::byte> request_body_buf;

	std::cout << "here1" << std::endl;

	if (formRequestHeader(client_fd, request_header, request_body_buf) == ConnectionStatus::CLOSE)
		return ConnectionStatus::CLOSE;

	if (!clients[client_fd].getResponse().empty())
		return ConnectionStatus::OPEN;

	// std::cout << "body message buf: ";
	// for (const auto &byte : request_body_buf)
	// 	std::cout << static_cast<char>(byte);
	// std::cout << std::endl;

	std::cout << "request_header: " << request_header << std::endl;

	Request request(request_header);

	if (request.bodyExpected())
	{
		if (formRequestBody(client_fd, request_body_buf, request) == ConnectionStatus::CLOSE)
			return ConnectionStatus::CLOSE;
	}

	Response response(request);

	std::string body;

	for (std::byte byte : response.getBody())
		body.push_back(static_cast<char>(byte));

	clients[client_fd].setResponse(response.getHeader().append(body));
	return ConnectionStatus::OPEN;
}

Server::ConnectionStatus Server::formRequestHeader(int const &client_fd, std::string &request_header, std::vector<std::byte> &request_body_buf)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];
	std::string delimitor = "\r\n\r\n";

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		request_header.append(buf, bytes);
		size_t delimitor_pos = request_header.find(delimitor);
		if (delimitor_pos != std::string::npos)
		{
			// for (char ch : request_header.substr(delimitor_pos + delimitor.length()))
			// {
			// 	request_body_buf.push_back(static_cast<std::byte>(ch));
			// 	std::cout << "ch: " << ch << " ";
			// }
			for (size_t i = delimitor_pos + delimitor.length(); i < request_header.size() - 2; ++i)
			{ // store the message body that is already read into buf
				char ch = request_header[i];
				request_body_buf.push_back(static_cast<std::byte>(ch));
				std::cout << "ch: " << ch << " ";
			}
			std::cout << std::endl;
			request_header.erase(delimitor_pos);
			return ConnectionStatus::OPEN;
		}
	}
	if (errno == EWOULDBLOCK || errno == EAGAIN) // if can't search for the delimitor, send error to client
	{
		clients[client_fd].setResponse("no delimitor in request header"); // TODO - replace with response to client
		perror("no delimitor in request header");
		return ConnectionStatus::OPEN;
	}
	else if (bytes < 0 && errno != EINTR && errno != ECONNRESET && errno != ETIMEDOUT)
		throw RecvException();
	else // client has shutdown or timeout or interrupted by a signal , close connection, remove fd and remove client
		clients.erase(client_fd);

	return ConnectionStatus::CLOSE;
}

Server::ConnectionStatus Server::formRequestBody(int const &client_fd, std::vector<std::byte> &request_body_buf, Request &request)
{
	request.appendToBody(request_body_buf);

	ssize_t bytes;
	char buf[BUFFER_SIZE];
	size_t len = request.getContentLength();

	while (len > 0 && (bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		std::cout << buf << std::endl;
		std::vector<std::byte> newBodyChunk;
		for (char ch : buf)
			newBodyChunk.push_back(static_cast<std::byte>(ch));
		request.appendToBody(newBodyChunk);
		len -= bytes;
	}
	if (errno == EWOULDBLOCK || errno == EAGAIN) // read till the end
	{
		std::cout << "here" << std::endl;
		return ConnectionStatus::OPEN;
	}
	else if (bytes < 0 && errno != EINTR && errno != ECONNRESET && errno != ETIMEDOUT)
		throw RecvException();
	else // client has shutdown or timeout or interrupted by a signal , close connection, remove fd and remove client
		clients.erase(client_fd);

	return ConnectionStatus::CLOSE;
}

// send the response
Server::ConnectionStatus Server::sendResponse(int const &client_fd)
{
	std::string response = clients[client_fd].getResponse();

	// send response
	send(client_fd, response.c_str(), response.length(), 0);

	std::cout << "Response sent from server" << std::endl;

	// keep the connection by default
	return ConnectionStatus::OPEN;
	// if request header = close, close connection, remove fd and remove client
	// clients.erase(client_fd);
	// return ConnectionStatus::CLOSE;
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

std::vector<int> Server::getClinetsFd(void) const
{
	std::vector<int> client_fds;
	client_fds.reserve(clients.size());

	for (std::pair<int, Client> client : clients)
		client_fds.push_back(client.first);

	return client_fds;
}

const char *Server::SocketCreationException::what() const throw()
{
	return "Server::Fail to create socket";
}

const char *Server::SocketBindingException::what() const throw()
{
	return "Server::Fail to bind socket";
}

const char *Server::SocketListenException::what() const throw()
{
	return "Server::Fail to set socket in passive mode";
}

const char *Server::SocketSetNonBlockingException::what() const throw()
{
	return "Server::Fail to set socket as non-blocking";
}

const char *Server::SocketSetOptionException::what() const throw()
{
	return "Server::Fail to set socket options";
}

const char *Server::AcceptException::what() const throw()
{
	return "Server::accept() failed";
}

const char *Server::TimeoutException::what() const throw()
{
	return "Server::Operation timeout";
}

const char *Server::RecvException::what() const throw()
{
	return "Server::recv() failed";
}

const char *Server::SendException::what() const throw()
{
	return "Server::send() failed";
}
