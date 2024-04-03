#include "ServerManager.hpp"
#include <cstring>
#include <algorithm>

volatile sig_atomic_t shutdown_flag = 0; // flag for shutting down the server

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
		pollfds = rhs.pollfds;
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

	Server::configData_t config;
	config.serverPort = 8080;
	config.serverHost = "127.0.0.1";
	configs.push_back(config);
	config.serverPort = 8081;
	config.serverHost = "127.0.0.1";
	configs.push_back(config);

	//-------------------------------------------------------------

	signal(SIGINT, signalHandler);

	createServers();

	startServerLoop();

	for (const pollfd &fd : pollfds) // close all pollfds
		close(fd.fd);
}

void ServerManager::createServers()
{
	for (Server::configData_t &config : configs)
	{
		servers.emplace_back(config);								  // construct and insert server
		servers.back().setUpServerSocket();							  // set up each server socket
		pollfds.push_back({servers.back().getServerFd(), POLLIN, 0}); // add the server socket to poll fd
	}
}

// start the main server loop
void ServerManager::startServerLoop()
{
	while (!shutdown_flag)
	{
		handlePoll();
		for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it) // loop through all pollfds to check which certain events have occurred
		{
			if (shutdown_flag)
				break;
			else if (!it->revents)
				continue;
			else if (it->revents == POLLIN)
				handleReadyToRead(it);
			else if (it->revents == POLLOUT)
				handleReadyToWrite(it);
			else
				throw ReventErrorFlagException();
		}
	}
}

// copy list to vector for poll, and then move vector back to list
void ServerManager::handlePoll()
{
	std::vector<pollfd> pollfds_tmp(pollfds.begin(), pollfds.end()); // create a vector to hold the pollfds temporarily

	if (poll(pollfds_tmp.data(), pollfds_tmp.size(), -1) < 0) // call poll using the vector's data
	{
		if (errno == EINTR)
			return;
		throw PollException();
	}

	pollfds.clear();
	pollfds.insert(pollfds.end(), pollfds_tmp.begin(), pollfds_tmp.end()); // move the contents back from the vector
}

void ServerManager::handleReadyToRead(std::list<pollfd>::iterator &it)
{
	for (Server &server : servers)
	{
		if (server.getServerFd() == it->fd) // if the fd is server fd, accept new connection
		{
			for (const int &client_fd : server.acceptNewConnections())
				pollfds.push_back({client_fd, POLLIN, 0}); // add the new fd to poll fd
			break;
		}
		if (server.isClient(it->fd)) // if the fd is client fd of that server, parse and build response
		{
			if (server.receiveRequest(it->fd) == Server::ConnectionStatus::OPEN)
				*it = {it->fd, POLLOUT, 0}; // set fd to ready for write
			else							// if connection has been closed by the client, close connection and remove fd
			{
				close(it->fd);
				it = pollfds.erase(it);
			}
			break;
		}
	}
}

void ServerManager::handleReadyToWrite(std::list<pollfd>::iterator &it)
{
	for (Server &server : servers)
	{
		if (server.isClient(it->fd)) // if the fd is client fd of that server
		{
			if (server.sendResponse(it->fd) == Server::ConnectionStatus::OPEN) // keep the connection open by default
				*it = {it->fd, POLLIN, 0};									   // set fd to ready for read
			else															   // if request header = close, close connection and remove fd
			{
				close(it->fd);
				it = pollfds.erase(it);
			}
			break;
		}
	}
}

void ServerManager::handleException(std::exception &e)
{
	// TODO - send server error as response if the client is connected to the server
	for (const pollfd &fd : pollfds) // close all pollfds
		close(fd.fd);
	std::cout << e.what() << std::endl;
}

const char *ServerManager::PollException::what() const throw()
{
	return ("ServerManager::poll() failed");
}

const char *ServerManager::ReventErrorFlagException::what() const throw()
{
	return ("ServerManager::error flag in revent");
}

// std::cout << "pollfd :";
// for (auto &pollfd : pollfds)
// {
// 	std::cout << pollfd.fd << ", ";
// }
// std::cout << std::endl;