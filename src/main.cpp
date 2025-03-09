#include "Server/ServerManager.hpp"
#include "Config/ConfigParser.hpp"
#include "Utils/Logger.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0];
		std::cout << " <fileName>\n";
		return 0;
	}
	std::string fileName = argv[1];
	ServerManager server_manager;
	try
	{
		Logger::initLogger();
		ConfigParser parser(fileName);
		parser.extractServerConfigs();
		// parser.printCluster(); // debug
		server_manager.initServer(parser.getServerConfigs());
	}
	catch (std::exception &e)
	{
		std::cerr << RED "[Invalid config file] " << e.what() << RESET << std::endl;
		return 1;
	}
	return server_manager.runServer();
}