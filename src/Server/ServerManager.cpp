#include "ServerManager.hpp"

ServerManager* serverManagerPtr = nullptr;

void interruptHandler(int signum)
{
	(void)signum;
	if (serverManagerPtr != nullptr)
	{
		Logger::log(e_log_level::INFO, SERVER, "Interrupt signal received");
		serverManagerPtr->cleanUpForServerShutdown(HttpStatusCode::INTERNAL_SERVER_ERROR);
		std::exit(EXIT_FAILURE);
	}
}

void segfaultHandler(int signum)
{
	(void)signum;
	if (serverManagerPtr != nullptr)
	{
		Logger::log(e_log_level::ERROR, ERROR_MESSAGE, "Seg fault");
		Logger::log(e_log_level::ERROR, ERROR_MESSAGE, "Internal server error");
		serverManagerPtr->cleanUpForServerShutdown(HttpStatusCode::INTERNAL_SERVER_ERROR);
		std::exit(EXIT_FAILURE);
	}
}

void ServerManager::initServer(const std::vector<ConfigData> &sparsedConfigs)
{
	serverConfigs = sparsedConfigs;
}

// run the server
int ServerManager::runServer()
{
	serverManagerPtr = this;
	signal(SIGINT, interruptHandler);
	signal(SIGSEGV, segfaultHandler);

	try
	{
		createServers();
		startServerLoop();
		return EXIT_SUCCESS;
	}
	catch (std::exception &e)
	{
		Logger::log(e_log_level::ERROR, ERROR_MESSAGE, "Internal server error - %s", e.what());
		cleanUpForServerShutdown(HttpStatusCode::INTERNAL_SERVER_ERROR);
		return EXIT_FAILURE;
	}
}

void ServerManager::createServers()
{
	for (ConfigData &config : serverConfigs)
	{
		Server server(config);
		server.setUpServerSocket(); // set up each server socket
		Logger::log(e_log_level::INFO, SERVER, "Server %s created - Host: %s, Port: %d",
			config.getServerName().c_str(),
			config.getServerHost().c_str(),
			config.getServerPort());
		servers[server.getServerFd()] = std::move(server);	  // insert server into map
		pollfds.push_back({server.getServerFd(), POLLIN, 0}); // add the server socket to poll fd
	}
}

// start the main server loop
void ServerManager::startServerLoop()
{
	while (true)
	{
		// std::cout << "before poll" << std::endl;
		handlePoll();
		// std::cout << "after poll" << std::endl;
		for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it) // loop through all pollfds to check which events have occurred
		{
			// std::cout << "poll loop fd:" << it->fd << std::endl;
			if (!it->revents)
			{
				// std::cout << "no revent" << std::endl;
				continue;
			}
			else if (it->revents & POLLIN)
			{
				// std::cout << "POLLIN" << std::endl;
				handleReadyToRead(it);
			}
			else if (it->revents & POLLOUT)
			{
				// std::cout << "POLLOUT" << std::endl;
				handleReadyToWrite(it);
			}
			else if (it->revents & POLLHUP && client_to_server_map.find(it->fd) != client_to_server_map.end())
			{
				// std::cout << "POLLHUP on client" << std::endl;
				int client_fd = it->fd;
				int server_fd = client_to_server_map[client_fd];
				Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d timeout",
					inet_ntoa(servers[server_fd].getClientIPv4Address(client_fd)),
					ntohs(servers[server_fd].getClientPortNumber(client_fd)));
				handleClientDisconnection(it);
			}
			else
			{
				// std::cout << "throw ReventErrorFlagException" << std::endl;
				throw ReventErrorFlagException();
			}
		}
	}
}

// copy list to vector for poll, and then move vector back to list
void ServerManager::handlePoll()
{
	std::vector<pollfd> pollfds_tmp(pollfds.begin(), pollfds.end()); // create a vector to hold the pollfds temporarily

	int ready = poll(pollfds_tmp.data(), pollfds_tmp.size(), TIMEOUT); // call poll using the vector's data

	pollfds.clear();
	pollfds.insert(pollfds.end(), pollfds_tmp.begin(), pollfds_tmp.end()); // move the contents back from the vector

	// std::cout << "fds: ";
	// for (auto &pollfd : pollfds)
	// 	std::cout << pollfd.fd << " ";
	// std::cout << std::endl;

	if (ready < 0)
		throw PollException();
	else // check timeout for each client socket
		checkClientTimeout(ready);
}

// disconnect client socket if it timeout or poll timeout
void ServerManager::checkClientTimeout(int const &ready)
{
	for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it)
	{
		if (client_to_server_map.find(it->fd) != client_to_server_map.end())
		{
			std::chrono::duration<double> elapsed_seconds = std::chrono::steady_clock::now() - client_last_active_time[it->fd];
			if (ready == 0 || elapsed_seconds.count() >= TIMEOUT / 1000) // if poll timeout or the client timeout
			{
				int client_fd = it->fd;
				int server_fd = client_to_server_map[client_fd];
				Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d timeout",
					inet_ntoa(servers[server_fd].getClientIPv4Address(client_fd)),
					ntohs(servers[server_fd].getClientPortNumber(client_fd)));
				servers[server_fd].createAndSendErrorResponse(REQUEST_TIMEOUT, client_fd);
				handleClientDisconnection(it);
			}
		}
	}
}

void ServerManager::handleReadyToRead(std::list<pollfd>::iterator &it)
{
	if (servers.find(it->fd) != servers.end()) // if the fd is server fd, accept new connection
	{
		int server_fd = it->fd;
		std::vector<int> clients_fds = servers[server_fd].acceptNewConnections();
		for (const int &client_fd : clients_fds)
		{
			pollfds.push_back({client_fd, POLLIN, 0});	 // add the new fd to poll fd
			client_to_server_map[client_fd] = server_fd; // add client fd to client to server map
			client_last_active_time[client_fd] = std::chrono::steady_clock::now();
		}
	}
	else // if the fd is client fd, parse and build response
	{
		int client_fd = it->fd;
		int server_fd = client_to_server_map[client_fd];
		Server::RequestStatus request_status = servers[server_fd].receiveRequest(client_fd);
		client_last_active_time[client_fd] = std::chrono::steady_clock::now();
		if (request_status == Server::REQUEST_DISCONNECT_CLIENT)
			handleClientDisconnection(it);
		else if (request_status == Server::READY_TO_WRITE)
			*it = {client_fd, POLLOUT, 0};
	}
}

void ServerManager::handleReadyToWrite(std::list<pollfd>::iterator &it)
{
	int client_fd = it->fd;
	int server_fd = client_to_server_map[client_fd];
	Server::ResponseStatus response_status = servers[server_fd].sendResponse(client_fd);
	client_last_active_time[client_fd] = std::chrono::steady_clock::now();
	if (response_status == Server::KEEP_ALIVE)
		*it = {client_fd, POLLIN, 0};
	else if (response_status == Server::RESPONSE_INTERRUPTED)
		return;
	else
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
	client_last_active_time.erase(client_fd);
	it = pollfds.erase(it);
	it--;
	Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d is removed",
		inet_ntoa(servers[server_fd].getClientIPv4Address(client_fd)),
		ntohs(servers[server_fd].getClientPortNumber(client_fd)));
}

// if the server shutdown, send error response to all clients and close all clients
void ServerManager::cleanUpForServerShutdown(HttpStatusCode const &statusCode)
{
	for (auto & pair :client_to_server_map)
		servers[pair.second].createAndSendErrorResponse(statusCode, pair.first);
	for (const pollfd &fd : pollfds) // close all pollfds
		close(fd.fd);
	for (auto & server: servers)
		Logger::log(e_log_level::INFO, SERVER, "Server %s shut down", server.second.getConfig().getServerName().c_str());
}

const char *ServerManager::PollException::what() const throw()
{
	return "ServerManager::poll() failed";
}

const char *ServerManager::ReventErrorFlagException::what() const throw()
{
	return "ServerManager::error flag in revent";
}
