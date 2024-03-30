#include "Server.hpp"
#include <cstring>

Server::Server(const std::unordered_map<std::string, std::string> &config) : config(config)
{
	client.addrlen = sizeof(client.address);
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
		client_fds = rhs.client_fds;
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

	server_fd = socket(AF_INET, SOCK_STREAM, 0); // create socket file descriptor
	if (server_fd < 0)
		throw SocketCreationException();

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, // set file descriptor to be reuseable
								 sizeof(opt)) < 0)
		throw SocketSetOptionException();

	if (fcntl(server_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
		throw SocketSetNonBlockingException();

	address.sin_family = AF_INET;
	address.sin_port = htons(std::stoi(config["port"]));
	address.sin_addr.s_addr = inet_addr(config["host"].c_str());

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) // bind the socket to the address and port number
		throw SocketBindingException();

	if (listen(server_fd, BACKLOG) < 0) // set server socket in passive mode
		throw SocketListenException();
}

// accept new connection(s)
std::vector<int> Server::acceptNewConnection()
{
	std::vector<int> new_fds;
	int new_fd = -1;
	do
	{
		new_fd = accept(server_fd,
										(struct sockaddr *)&(client.address),
										&(client.addrlen));

		if (new_fd < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // listen() queue is empty
				break;
			throw AcceptException();
		}
		client_fds.push_back(new_fd);
		new_fds.push_back(new_fd);
	} while (new_fd != -1);

	return new_fds;
}

// receive the request
Server::ConnectionStatus Server::receiveRequest(int const &client_fd)
{

	char buf[BUFFER_SIZE];
	int rc = recv(client_fd, buf, sizeof(buf), 0);
	if (rc == 0) // connection has been closed by the client
		return ConnectionStatus::CLOSE;
	if (rc < 0)
	{
		if (errno != EWOULDBLOCK || errno != EAGAIN)
			throw RecvException();
	}
	// read until /r/n
	// Request request(headerStr);
	// if (request.success)
	// {
	// read body for request.bytes
	//	request.addBody(bodyStr);
	// }
	// Response (request);
	// save response to client
	std::cout << buf << std::endl;
	return ConnectionStatus::OPEN;
}

// send the response
Server::ConnectionStatus Server::sendResponse(int const &client_fd)
{
	// send response
	char buf[1024] = "Hello from server";
	send(client_fd, buf, sizeof(buf), 0);

	std::cout << "Hello from server" << std::endl;

	// keep the connection by default
	// return ConnectionStatus::OPEN;
	// if request header = close, remove fd and close connection
	return ConnectionStatus::CLOSE;
}

int const &Server::getServerFd(void) const
{
	return server_fd;
}

std::vector<int> const &Server::getClientFds(void) const
{
	return client_fds;
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
	return ("Server::Accept() failed");
}

const char *Server::RecvException::what() const throw()
{
	return ("Server::Recv() failed");
}

const char *Server::SendException::what() const throw()
{
	return ("Server::Send() failed");
}
