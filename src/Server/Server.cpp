#include "Server.hpp"

volatile sig_atomic_t shutdown_flag = 0;

Server::Server()
{
	client.addrlen = sizeof(client.address);
	// to be replaced by config file
	std::unordered_map<std::string, std::string> server;
	server["port"] = "8082";
	server["host"] = "127.0.0.1";
	servers.push_back(server);
	server["port"] = "8083";
	server["host"] = "127.0.0.1";
	servers.push_back(server);
}

Server::~Server()
{
}

// signal handler for shutting down the server
void signalHandler(int signum)
{
	shutdown_flag = 1;
}

// run the server
void Server::runServer()
{
	// TODO - create Config object
	// register signal handler
	signal(SIGINT, signalHandler);
	setUpServerSocket();
	serverLoop();
	// close all fds
	for (pollfd &fd : fds)
		close(fd.fd);
}

// set up server socket
void Server::setUpServerSocket()
{
	int server_fd;
	struct sockaddr_in address;
	int opt;

	opt = 1;
	for (auto &server : servers)
	{
		// create socket file descriptor
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0)
			throw SocketCreationException();
		// add server socket and its info into server_sockets map
		server_sockets[server_fd] = server;

		// set file descriptor to be reuseable
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
					   sizeof(opt)) < 0)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketSetOptionException();
		}

		/* Set socket to be nonblocking. All of the sockets for
		the incoming connections will also be nonblocking since
		they will inherit that state from the listening socket. */
		if (fcntl(server_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketSetNonBlockingException();
		}

		// define the address and port number
		address.sin_family = AF_INET;
		address.sin_port = htons(std::stoi(server["port"]));
		address.sin_addr.s_addr = inet_addr(server["host"].c_str());

		// bind the socket to the address and port number
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketBindingException();
		}

		// set server socket in passive mode
		if (listen(server_fd, BACKLOG) < 0)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketListenException();
		}

		// add the server sockets to poll fd
		fds.push_back({server_fd, POLLIN, 0});
	}
}

// main loop for server
void Server::serverLoop()
{
	while (!shutdown_flag)
	{
		// wait for event on a file descriptor
		if (poll(fds.data(), fds.size(), -1) < 0)
		{
			if (!shutdown_flag)
			{
				for (pollfd &fd : fds)
					close(fd.fd);
				throw PollException();
			}
		}

		// loop through all fds to check the event
		for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
		{
			if (it->revents & POLLIN)
			{
				bool isServer = false;

				for (auto &server_socket : server_sockets)
				{
					if (it->fd == server_socket.first)
						isServer = true;
				}

				if (isServer == true)
					acceptNewConnection(*it); // accept new connection if the fd is server fd
				else
					recvRequest(it); // parse and build response

				break;
			}
			else if (it->revents & POLLOUT)
			{
				sendResponse(it);
				break;
			}
		}
	}
}

// accept new connection
void Server::acceptNewConnection(pollfd &fd)
{
	int new_fd = accept(fd.fd,
						(struct sockaddr *)&(client.address),
						&(client.addrlen));
	if (new_fd < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw AcceptException();
		}
	}
	// add the new fd to poll fd
	fds.push_back({new_fd, POLLIN, 0});
}

// receive the request
void Server::recvRequest(std::vector<pollfd>::iterator it)
{
	char buf[BUFFER_SIZE];
	recv(it->fd, buf, sizeof(buf), 0);
	std::cout << buf << std::endl;
	// set fd to ready for write
	*it = {it->fd, POLLOUT, 0};
}

// send the response
void Server::sendResponse(std::vector<pollfd>::iterator it)
{
	// send response
	char buf[1024] = "Hello from server";
	//  write(fd.fd, buf, sizeof(buf));
	send(it->fd, buf, sizeof(buf), 0);
	printf("Hello message sent\n");

	// if request header = keep alive
	//  *it = {it->fd, POLLIN, 0};
	// else remove fd and close connection

	close(it->fd);
	fds.erase(it);
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

const char *Server::PollException::what() const throw()
{
	return ("Server::Poll() failed");
}

const char *Server::AcceptException::what() const throw()
{
	return ("Server::Accept() failed");
}
