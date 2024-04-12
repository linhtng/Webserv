#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sys/poll.h>
#include <csignal>
#include <iostream>
#include <list>
#include <unistd.h>
#include "Server.hpp"

#define POLL_TIMEOUT 100000

typedef typename std::vector<Server::configData_t> config_t;

class ServerManager
{

private:
	//--------------------------------------------------------------
	// TODO - to be replaced by config file
	config_t configs;
	//--------------------------------------------------------------

	std::unordered_map<int, Server> servers;
	std::unordered_map<int, int> client_to_server_map;
	std::list<pollfd> pollfds;

	void createServers();
	void startServerLoop();
	void handlePoll();
	void handleReadyToRead(std::list<pollfd>::iterator &it);
	void handleReadyToWrite(std::list<pollfd>::iterator &it);
	void handleClientDisconnection(std::list<pollfd>::iterator &it);
	void handleServerError(const int &status_code);

public:
	ServerManager();
	ServerManager(ServerManager const &src);
	ServerManager &operator=(ServerManager const &rhs);
	~ServerManager();

	int runServer();

	class PollException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class ReventErrorFlagException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};
};

#endif
