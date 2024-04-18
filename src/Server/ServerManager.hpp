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
#include <chrono>
#include "Server.hpp"
#include "../config_parser/ConfigParser.hpp"

#define TIMEOUT 10000

typedef typename std::vector<Server::configData_t> config_t;

class ServerManager
{

private:
	//--------------------------------------------------------------
	// TODO - to be replaced by config file
	std::vector<ConfigData> serverConfigs;
	//--------------------------------------------------------------

	std::unordered_map<int, Server>
		servers;
	std::unordered_map<int, int> client_to_server_map;
	std::list<pollfd> pollfds;
	std::unordered_map<int, std::chrono::steady_clock::time_point> client_last_active_time;

	void createServers();
	void startServerLoop();
	void handlePoll();
	void handlePollTimeout();
	void checkClientTimeout();
	void handleReadyToRead(std::list<pollfd>::iterator &it);
	void handleReadyToWrite(std::list<pollfd>::iterator &it);
	void handleClientDisconnection(std::list<pollfd>::iterator &it);
	void cleanUpForServerShutdown(const int &status_code);

public:
	ServerManager();
	ServerManager(ServerManager const &src);
	ServerManager &operator=(ServerManager const &rhs);
	~ServerManager();

	void initServer(const std::vector<ConfigData> &parsedConfigs);
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
