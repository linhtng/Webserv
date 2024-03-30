#include "ServerManager.hpp"
#include <cstring>
#include <algorithm>

volatile sig_atomic_t shutdown_flag = 0;

ServerManager::ServerManager()
{
}

ServerManager::ServerManager(ServerManager const &src)
{
	*this = src;
}

ServerManager &ServerManager::operator=(ServerManager const &rhs)
{
	if (this != &rhs)
	{
		configs = rhs.configs;
		servers = rhs.servers;
		fds = rhs.fds;
	}

	return *this;
}

ServerManager::~ServerManager()
{
}

// signal handler for shutting down the server
void signalHandler(int signum)
{
	(void)signum;
	shutdown_flag = 1;
}

// run the server
void ServerManager::runServer()
{

	//--------------------------------------------------------------
	// TODO - create Config object and to be replaced by config file

	std::unordered_map<std::string, std::string> config;
	config["port"] = "8082";
	config["host"] = "127.0.0.1";
	configs.push_back(config);
	config.clear();
	config["port"] = "8083";
	config["host"] = "127.0.0.1";
	configs.push_back(config);

	//-------------------------------------------------------------

	signal(SIGINT, signalHandler);

	createServers();

	startServerLoop(); // start the main server loop

	for (pollfd &fd : fds) // close all fds
		close(fd.fd);
}

void ServerManager::createServers()
{
	for (std::unordered_map<std::string, std::string> &config : configs) // construct and insert server
	{
		Server server(config);
		try
		{
			server.setUpServerSocket(); // set up each server socket
		}
		catch (std::exception &e)
		{
			handleServerError(e);
		}
		servers.push_back(server);
		fds.push_back({server.getServerFd(), POLLIN, 0}); // add the server socket to poll fd
	}
}

// main loop for server
void ServerManager::startServerLoop()
{
	while (!shutdown_flag)
	{
		setUpPoll();
		for (std::list<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) // loop through all fds to check the event
		{
			if (!it->revents)
				continue;
			else if (it->revents == POLLIN)
				handlePollin(it);
			else if (it->revents == POLLOUT)
				handlePollout(it);
			else
			{
				for (pollfd &fd : fds)
					close(fd.fd);
				throw PollErrorException();
			}
		}
	}
}

// copy list to vector for poll, and then move vector back to list
void ServerManager::setUpPoll()
{
	std::vector<pollfd> poll_fds;
	poll_fds.reserve(fds.size());
	std::copy(fds.begin(), fds.end(), std::back_inserter(poll_fds)); // copy list to vector

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
	std::move(poll_fds.begin(), poll_fds.end(), std::back_inserter(fds)); // move vector to list
}

void ServerManager::handlePollin(std::list<pollfd>::iterator &it)
{
	for (Server &server : servers)
	{
		if (server.getServerFd() == it->fd) // if the fd is server fd
		{
			try
			{
				for (int &new_fd : server.acceptNewConnection()) // accept new connection
					fds.push_back({new_fd, POLLIN, 0});						 // add the new fd to poll fd
			}
			catch (std::exception &e)
			{
				handleServerError(e);
			}
			break;
		}
		if (std::find(server.getClientFds().begin(), server.getClientFds().end(), it->fd) != server.getClientFds().end()) // if the fd is client fd of that server
		{
			try
			{																																				// parse and build response
				if (server.receiveRequest(it->fd) == Server::ConnectionStatus::CLOSE) // if connection has been closed by the client, close connection and remove fd
				{
					close(it->fd);
					it = fds.erase(it);
				}
				else
					*it = {it->fd, POLLOUT, 0}; // set fd to ready for write
			}
			catch (std::exception &e)
			{
				handleServerError(e);
			}
			break;
		}
	}
}

void ServerManager::handlePollout(std::list<pollfd>::iterator &it)
{
	for (Server &server : servers)
	{
		if (std::find(server.getClientFds().begin(), server.getClientFds().end(), it->fd) != server.getClientFds().end()) // if the fd is client fd of that server
		{
			if (server.sendResponse(it->fd) == Server::ConnectionStatus::OPEN) // keep the connection open by default
				*it = {it->fd, POLLIN, 0};																			 // set fd to ready for read
			else																															 // if request header = close, close connection and remove fd
			{
				close(it->fd);
				it = fds.erase(it);
			}
			break;
		}
	}
}

void ServerManager::handleServerError(std::exception &e)
{
	for (pollfd &fd : fds) // close all fds
		close(fd.fd);
	std::cout << e.what() << std::endl;
	exit(EXIT_FAILURE);
}

const char *ServerManager::PollException::what() const throw()
{
	return ("ServerManager::Poll() failed");
}

const char *ServerManager::PollErrorException::what() const throw()
{
	return ("ServerManager::POLL error");
}
