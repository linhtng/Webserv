#include "Server.hpp"
#include <cstring>

volatile sig_atomic_t shutdown_flag = 0;

Server::Server()
{
	client.addrlen = sizeof(client.address);

	// TODO - to be replaced by config file

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
	(void)signum;
	shutdown_flag = 1;
}

// run the server
void Server::runServer()
{
	// TODO - create Config object

	signal(SIGINT, signalHandler); // register signal handler
	setUpServerSocket();
	serverLoop();

	for (pollfd &fd : fds) // close all fds
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
		server_fd = socket(AF_INET, SOCK_STREAM, 0); // create socket file descriptor
		if (server_fd < 0)
			throw SocketCreationException();

		server_sockets[server_fd] = server; // add server socket and its info into server_sockets map

		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, // set file descriptor to be reuseable
									 sizeof(opt)) < 0)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketSetOptionException();
		}

		if (fcntl(server_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketSetNonBlockingException();
		}

		address.sin_family = AF_INET; // define the address and port number
		address.sin_port = htons(std::stoi(server["port"]));
		address.sin_addr.s_addr = inet_addr(server["host"].c_str());

		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) // bind the socket to the address and port number
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketBindingException();
		}

		if (listen(server_fd, BACKLOG) < 0) // set server socket in passive mode
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw SocketListenException();
		}

		fds.push_back({server_fd, POLLIN, 0}); // add the server socket to poll fd
	}
}

// main loop for server
void Server::serverLoop()
{
	while (!shutdown_flag)
	{
		// std::cout << "set: ";
		// for (auto &fd : fds)
		// {
		// 	std::cout << fd.fd << ", ";
		// }
		// std::cout << std::endl;

		std::vector<pollfd> poll_fds;
		poll_fds.reserve(fds.size());
		std::copy(fds.begin(), fds.end(), std::back_inserter(poll_fds));

		if (poll(poll_fds.data(), fds.size(), -1) < 0) // wait for event on a file descriptor
		{
			if (!shutdown_flag)
			{
				for (pollfd &fd : fds)
					close(fd.fd);
				throw PollException();
			}
		}

		fds.clear();
		std::move(poll_fds.begin(), poll_fds.end(), std::back_inserter(fds));

		// std::cout << "new set: ";
		// for (auto &fd : fds)
		// {
		// 	std::cout << fd.fd << ", ";
		// }
		// std::cout << std::endl;

		for (std::list<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) // loop through all fds to check the event
		{
			// std::cout << "fd: " << it->fd << std::endl;

			if (!it->revents)
				continue;
			else if (it->revents == POLLIN)
			{
				if (server_sockets.find(it->fd) != server_sockets.end())
					acceptNewConnection(it->fd); // accept new connection if the fd is server fd
				else
					receiveRequest(it); // parse and build response
			}
			else if (it->revents == POLLOUT)
				sendResponse(it);
			else
			{
				for (pollfd &fd : fds)
					close(fd.fd);
				throw PollErrorException();
			}
		}
	}
}

// accept new connection(s)
void Server::acceptNewConnection(const int &server_fd)
{
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

			for (pollfd &fd : fds)
				close(fd.fd);
			throw AcceptException();
		}

		fds.push_back({new_fd, POLLIN, 0}); // add the new fd to poll fd
	} while (new_fd != -1);
}

// receive the request
void Server::receiveRequest(std::list<pollfd>::iterator &it)
{

	char buf[BUFFER_SIZE];
	int rc = recv(it->fd, buf, sizeof(buf), 0);
	if (rc == 0) // connection has been closed by the client
	{
		close(it->fd);
		it = fds.erase(it);
	}

	if (rc < 0)
	{
		if (errno != EWOULDBLOCK || errno != EAGAIN)
		{
			for (pollfd &fd : fds)
				close(fd.fd);
			throw RecvException();
		}
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

	*it = {it->fd, POLLOUT, 0}; // set fd to ready for write
}

// send the response
void Server::sendResponse(std::list<pollfd>::iterator &it)
{
	// send response
	char buf[1024] = "Hello from server";
	send(it->fd, buf, sizeof(buf), 0);

	std::cout << "Hello from server" << std::endl;

	// keep the connect by default
	// *it = {it->fd, POLLIN, 0};
	// if request header = close, remove fd and close connection
	close(it->fd);
	it = fds.erase(it);
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

const char *Server::RecvException::what() const throw()
{
	return ("Server::Recv() failed");
}

const char *Server::SendException::what() const throw()
{
	return ("Server::Send() failed");
}

const char *Server::PollErrorException::what() const throw()
{
	return ("Server::POLL error");
}
