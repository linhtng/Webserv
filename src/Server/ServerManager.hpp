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
#include "Server.hpp"

typedef typename std::vector<std::unordered_map<std::string, std::string>> config_t;
typedef typename std::unordered_map<int, std::unordered_map<std::string, std::string>> server_sockets_t;

class ServerManager
{

private:
	//--------------------------------------------------------------
	// TODO - to be replaced by config file
	config_t configs;
	//--------------------------------------------------------------

	std::vector<Server> servers;
	std::list<pollfd> pollfds;

	void createServers();
	void startServerLoop();
	void setUpPoll();
	void handlePollin(std::list<pollfd>::iterator &it);
	void handlePollout(std::list<pollfd>::iterator &it);
	void handleServerError(std::exception &e);

public:
	class PollException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	class PollErrorException : public std::exception
	{
	public:
		virtual const char *what() const throw();
	};

	ServerManager();
	ServerManager(ServerManager const &src);
	ServerManager &operator=(ServerManager const &rhs);
	~ServerManager();

	void runServer();
};

#endif
