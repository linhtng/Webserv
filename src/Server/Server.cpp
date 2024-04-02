#include "Server.hpp"
#include <cstring>

Server::Server(const std::unordered_map<std::string, std::string> &config) : server_fd(0), config(config)
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
		Client client;
		new_fd = accept(server_fd,
										(struct sockaddr *)&(client.getAndSetAddress()),
										&(client.getAndSetAddrlen()));
		if (new_fd < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // listen() queue is empty
				break;
			throw AcceptException();
		}
		client.setFd(new_fd);
		if (fcntl(new_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
			throw SocketSetNonBlockingException();
		clients.push_back(client);
		new_fds.push_back(new_fd);
	} while (new_fd != -1);

	return new_fds;
}

// receive the request
Server::ConnectionStatus Server::receiveRequest(int const &client_fd)
{
	char buf[BUFFER_SIZE];
	std::string request_header;
	size_t delimitor_pos;
	std::string delimitor = "\r\n\r\n";

	do
	{
		ssize_t bytes = recv(client_fd, buf, sizeof(buf), 0);
		if (bytes == 0) // connection has been closed by the client, close connection, remove fd and remove client
		{
			clients.erase(std::remove_if(clients.begin(), clients.end(), [client_fd](const Client &client)
																	 { return client.getClientFd() == client_fd; }),
										clients.end());
			return ConnectionStatus::CLOSE;
		}

		if (bytes < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // if no messages are available, send error to client
			{
				for (auto &client : clients)
				{
					if (client.getClientFd() == client_fd)
					{
						client.setResponse("timeout");
						break;
					}
				}
				perror("timeout "); // TODO - replace with response to client
				return ConnectionStatus::OPEN;
			}
			throw RecvException();
		}

		request_header.append(buf, bytes);
		delimitor_pos = request_header.find(delimitor);
	} while (delimitor_pos == std::string::npos); // read until \r\n\r\n

	std::string body_message_buf = request_header.substr(delimitor_pos + delimitor.length()); // store the message body that is already read into buf
	request_header.erase(delimitor_pos);

	std::cout << "body message buf: " << body_message_buf << std::endl;
	std::cout << "request_header: " << request_header << std::endl;

	// Request request(headerStr, config);
	// if (request.success)
	// {
	// body = body_message_buf + loop for read
	// read body for request.bytes
	//	request.addBody(bodyStr);
	// }
	// Response (request);
	// save response to client

	for (auto &client : clients)
	{
		if (client.getClientFd() == client_fd)
		{
			client.setResponse("Hello from server");
			break;
		}
	}

	return ConnectionStatus::OPEN;
}

// send the response
Server::ConnectionStatus Server::sendResponse(int const &client_fd)
{
	std::string response;

	for (auto &client : clients)
	{
		if (client.getClientFd() == client_fd)
		{
			response = client.getResponse();
			break;
		}
	}
	// send response
	send(client_fd, response.c_str(), response.length(), 0);

	std::cout << "Response sent from server" << std::endl;

	// keep the connection by default
	// return ConnectionStatus::OPEN;
	// if request header = close, close connection, remove fd and remove client
	clients.erase(std::remove_if(clients.begin(), clients.end(), [client_fd](const Client &client)
															 { return client.getClientFd() == client_fd; }),
								clients.end());
	return ConnectionStatus::CLOSE;
}

bool Server::isClient(int const &client_fd)
{
	return std::any_of(clients.begin(), clients.end(),
										 [client_fd](const Client &client)
										 {
											 return client.getClientFd() == client_fd;
										 });
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
	return ("Server::Accept() failed");
}

const char *Server::TimeoutException::what() const throw()
{
	return ("Server::Operation timeout");
}

const char *Server::RecvException::what() const throw()
{
	return ("Server::Recv() failed");
}

const char *Server::SendException::what() const throw()
{
	return ("Server::Send() failed");
}
