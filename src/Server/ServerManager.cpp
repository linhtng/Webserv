#include "ServerManager.hpp"

ServerManager *serverManagerPtr = nullptr;
volatile sig_atomic_t shutdownFlag = 0; // flag for shutting down the server

void interruptHandler(int signum)
{
	(void)signum;
	shutdownFlag = 1;
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

int ServerManager::runServer()
{
	serverManagerPtr = this;
	signal(SIGINT, interruptHandler);
	signal(SIGSEGV, segfaultHandler);

	try
	{
		createServers();
		startServerLoop();
		Logger::log(e_log_level::INFO, SERVER, "Interrupt signal received");
		cleanUpForServerShutdown(HttpStatusCode::INTERNAL_SERVER_ERROR);
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
		const std::pair<const int, std::unique_ptr<Server>> *serverPtr = findServer(config.getServerHost(), config.getServerPort());

		if (serverPtr == nullptr) // if it is a new server
		{
			std::unique_ptr<Server> server = std::make_unique<Server>(config);
			server->setUpServerSocket();
			Logger::log(e_log_level::INFO, SERVER, "Server created - Host: %s, Port: %d, Server Name: %s",
						server->getHost().c_str(),
						server->getPort(),
						config.getServerName().c_str());
			int serverFd = server->getServerFd();
			servers[serverFd] = std::move(server);	  // insert server into map
			pollfds.push_back({serverFd, POLLIN, 0}); // add the server socket to poll fd
		}
		else
		{
			serverPtr->second->appendConfig(config);
			Logger::log(e_log_level::INFO, SERVER, "Configuration of Server Name %s added to Server %s:%d",
						config.getServerName().c_str(),
						config.getServerHost().c_str(),
						config.getServerPort());
		}
	}
}

const std::pair<const int, std::unique_ptr<Server>> *ServerManager::findServer(const std::string &host, const int &port) const
{
	for (const std::pair<const int, std::unique_ptr<Server>> &server : servers)
	{
		if (server.second->getHost() == host && server.second->getPort() == port)
			return &server;
	}
	return nullptr;
}

// start the main server loop
void ServerManager::startServerLoop()
{
	while (!shutdownFlag)
	{
		handlePoll();
		for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end() && !shutdownFlag; ++it) // loop through all pollfds to check which events have occurred
		{
			if (!it->revents)
				continue;
			else if (it->revents & POLLIN)
				handleReadyToRead(it);
			else if (it->revents & POLLOUT)
				handleReadyToWrite(it);
			else if (it->revents & POLLHUP && clientToServerMap.find(it->fd) != clientToServerMap.end())
			{
				int clientFd = it->fd;
				int serverFd = clientToServerMap[clientFd];
				Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnect",
							inet_ntoa(servers[serverFd]->getClientIPv4Address(clientFd)),
							ntohs(servers[serverFd]->getClientPortNumber(clientFd)));
				handleClientDisconnection(it);
			}
			else
				throw ReventErrorFlagException();
		}
	}
}

// copy list to vector for poll, and then move vector back to list
void ServerManager::handlePoll()
{
	std::vector<pollfd> pollfdsTmp(pollfds.begin(), pollfds.end()); // create a vector to hold the pollfds temporarily

	int ready = poll(pollfdsTmp.data(), pollfdsTmp.size(), SERVER_TIMEOUT); // call poll using the vector's data

	pollfds.clear();
	pollfds.insert(pollfds.end(), pollfdsTmp.begin(), pollfdsTmp.end()); // move the contents back from the vector

	if (shutdownFlag == 1)
		return;

	if (ready < 0)
		throw PollException();
	else // check timeout for each client socket
		checkClientTimeout(ready);
}

// disconnect client socket if the client timeout or poll timeout
void ServerManager::checkClientTimeout(int const &ready)
{
	for (std::list<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it)
	{
		if (clientToServerMap.find(it->fd) != clientToServerMap.end())
		{
			std::chrono::duration<double> elapsedSeconds = std::chrono::steady_clock::now() - clientLastActiveTime[it->fd];
			if (ready == 0 || elapsedSeconds.count() >= SERVER_TIMEOUT / 1000) // if poll timeout or the client timeout
			{
				int clientFd = it->fd;
				int serverFd = clientToServerMap[clientFd];
				Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d timeout",
							inet_ntoa(servers[serverFd]->getClientIPv4Address(clientFd)),
							ntohs(servers[serverFd]->getClientPortNumber(clientFd)));
				servers[serverFd]->createAndSendErrorResponse(REQUEST_TIMEOUT, clientFd);
				handleClientDisconnection(it);
			}
		}
	}
}

void ServerManager::handleReadyToRead(std::list<pollfd>::iterator &it)
{
	if (servers.find(it->fd) != servers.end()) // if the fd is server fd, accept new connection
	{
		int serverFd = it->fd;
		int clientFd = servers[serverFd]->acceptNewConnection();
		if (clientFd >= 0)
		{
			pollfds.push_back({clientFd, POLLIN, 0}); // add the new client fd to poll fd
			clientToServerMap[clientFd] = serverFd;
			clientLastActiveTime[clientFd] = std::chrono::steady_clock::now();
		}
	}
	else // if the fd is client fd, parse the request and build response
	{
		int clientFd = it->fd;
		int serverFd = clientToServerMap[clientFd];
		Server::RequestStatus requestStatus = servers[serverFd]->receiveRequest(clientFd); // return REQUEST_CLIENT_DISCONNECT or READY_TO_WRITE or BODY_IN_CHUNK
		// after calling receiveRequest, now we have the clientCgiPipeFds map, how to push them to pollfds?
		std::pair<const int *, const int *> clientCgiFds = servers[serverFd]->getClientCgiPipeFds()[clientFd];
		int pipeIn = clientCgiFds.first[WRITE_END];
		int pipeOut = clientCgiFds.second[READ_END];
		if (pipeIn >= 0)
		{
			pollfds.push_back({pipeIn, POLLIN, 0});
			clientToServerMap[pipeIn] = serverFd;
			clientLastActiveTime[pipeIn] = std::chrono::steady_clock::now();
		}
		if (pipeOut >= 0)
		{
			pollfds.push_back({pipeOut, POLLIN, 0});
			clientToServerMap[pipeOut] = serverFd;
			clientLastActiveTime[pipeOut] = std::chrono::steady_clock::now();
		}
		clientLastActiveTime[clientFd] = std::chrono::steady_clock::now();
		if (requestStatus == Server::READY_TO_WRITE)
			*it = {clientFd, POLLOUT, 0};
		else if (requestStatus == Server::REQUEST_CLIENT_DISCONNECT)
			handleClientDisconnection(it);
	}
}

void ServerManager::handleReadyToWrite(std::list<pollfd>::iterator &it)
{
	int clientFd = it->fd;
	int serverFd = clientToServerMap[clientFd];
	Server::ResponseStatus responseStatus = servers[serverFd]->sendResponse(clientFd); // return RESPONSE_DISCONNECT_CLIENT or KEEP_ALIVE or RESPONSE_IN_CHUNK
	clientLastActiveTime[clientFd] = std::chrono::steady_clock::now();
	if (responseStatus == Server::KEEP_ALIVE)
		*it = {clientFd, POLLIN, 0};
	else if (responseStatus == Server::RESPONSE_DISCONNECT_CLIENT)
		handleClientDisconnection(it);
}

// if client's connection is closed, remove the client, close fd and remove fd
void ServerManager::handleClientDisconnection(std::list<pollfd>::iterator &it)
{
	int clientFd = it->fd;
	int serverFd = clientToServerMap[clientFd];
	servers[serverFd]->removeClient(clientFd);
	close(clientFd);
	clientToServerMap.erase(clientFd);
	clientLastActiveTime.erase(clientFd);
	it = pollfds.erase(it);
	it--;
}

void ServerManager::cleanUpForServerShutdown(HttpStatusCode const &statusCode)
{
	for (const std::pair<const int, int> &clientToServerPair : clientToServerMap) // send error response to all clients
		servers[clientToServerPair.second]->createAndSendErrorResponse(statusCode, clientToServerPair.first);
	for (const pollfd &fd : pollfds) // close all pollfds
		close(fd.fd);
	for (std::pair<const int, std::unique_ptr<Server>> &server : servers)
	{
		Logger::log(e_log_level::INFO, SERVER, "Server %s:%d shut down", server.second->getHost().c_str(), server.second->getPort());
		server.second.reset();
	}
}

const char *ServerManager::PollException::what() const throw()
{
	return "ServerManager::poll() failed";
}

const char *ServerManager::ReventErrorFlagException::what() const throw()
{
	return "ServerManager::error flag in revent";
}
