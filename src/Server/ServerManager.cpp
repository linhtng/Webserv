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
int ServerManager::runServer()
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

	try
	{
		createServers();
		startServerLoop();
		handleServerError(503);
		std::cout << "server shutting down..." << std::endl;
		return EXIT_SUCCESS;
	}
	catch (std::exception &e)
	{
		handleServerError(500);
		std::cout << "internal server error" << std::endl;
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}

void ServerManager::createServers()
{
	for (Server::configData_t &config : configs)
	{
		Server server(config);
		server.setUpServerSocket();							  // set up each server socket
		servers[server.getServerFd()] = std::move(server);	  // insert server
		pollfds.push_back({server.getServerFd(), POLLIN, 0}); // add the server socket to poll fd
	}
}

// start the main server loop
void ServerManager::startServerLoop()
{
	while (!shutdown_flag)
	{
		handlePoll();
		for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it) // loop through all pollfds to check which events have occurred
		{
			if (shutdown_flag)
				break;
			else if (!it->revents)
				continue;
			else if (it->revents & POLLIN)
				handleReadyToRead(it);
			else if (it->revents & POLLOUT)
				handleReadyToWrite(it);
			else if (it->revents & POLLHUP || it->revents & POLLERR)
				handleClientDisconnection(it);
			else
				throw ReventErrorFlagException();
		}
	}
}

// copy list to vector for poll, and then move vector back to list
void ServerManager::handlePoll()
{
	std::vector<pollfd> pollfds_tmp(pollfds.begin(), pollfds.end()); // create a vector to hold the pollfds temporarily

	int ready = poll(pollfds_tmp.data(), pollfds_tmp.size(), POLL_TIMEOUT); // call poll using the vector's data

	pollfds.clear();
	pollfds.insert(pollfds.end(), pollfds_tmp.begin(), pollfds_tmp.end()); // move the contents back from the vector

	if (ready < 0)
	{
		if (errno == EINTR)
			return;
		throw PollException();
	}
	else if (ready == 0) // timeout occur for all socket
	{
		for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it)
		{
			if (client_to_server_map.find(it->fd) != client_to_server_map.end()) // remove all client fd
				handleClientDisconnection(it);
		}
	}
}

void ServerManager::handleReadyToRead(std::list<pollfd>::iterator &it)
{
	if (servers.find(it->fd) != servers.end()) // if the fd is server fd, accept new connection
	{
		int server_fd = it->fd;
		for (const int &client_fd : servers[server_fd].acceptNewConnections())
		{
			pollfds.push_back({client_fd, POLLIN, 0});	 // add the new fd to poll fd
			client_to_server_map[client_fd] = server_fd; // add client fd to client to server map
		}
	}
	else // if the fd is client fd, parse and build response
	{
		int client_fd = it->fd;
		int server_fd = client_to_server_map[client_fd];
		if (servers[server_fd].receiveRequest(client_fd) == Server::ConnectionStatus::OPEN)
			*it = {client_fd, POLLOUT, 0}; // set fd to ready for write
		else							   // if connection has been closed by the client
			handleClientDisconnection(it);
	}
}

void ServerManager::handleReadyToWrite(std::list<pollfd>::iterator &it)
{
	int client_fd = it->fd;
	int server_fd = client_to_server_map[client_fd];
	if (servers[server_fd].sendResponse(client_fd) == Server::ConnectionStatus::OPEN)
		*it = {client_fd, POLLIN, 0}; // set fd to ready for read
	else							  // if request header = close
		handleClientDisconnection(it);
}

// if connection has been closed by the client, remove the client, close connection and remove fd
void ServerManager::handleClientDisconnection(std::list<pollfd>::iterator &it)
{
	int client_fd = it->fd;
	int server_fd = client_to_server_map[client_fd];
	servers[server_fd].removeClient(client_fd);
	close(client_fd);
	client_to_server_map.erase(client_fd);
	it = pollfds.erase(it);
}

void ServerManager::handleServerError(const int &status_code)
{
	std::string error_response = "server error with status code " + std::to_string(status_code);
	for (auto it = client_to_server_map.begin(); it != client_to_server_map.end(); ++it)
		send(it->first, error_response.c_str(), error_response.length(), 0); // TODO - replace with response to client
	for (const pollfd &fd : pollfds)										 // close all pollfds
		close(fd.fd);
}

const char *ServerManager::PollException::what() const throw()
{
	return "ServerManager::poll() failed";
}

const char *ServerManager::ReventErrorFlagException::what() const throw()
{
	return "ServerManager::error flag in revent";
}

// std::cout << "pollfd :";
// for (auto &pollfd : pollfds)
// {
// 	std::cout << pollfd.fd << ", ";
// }
// std::cout << std::endl;