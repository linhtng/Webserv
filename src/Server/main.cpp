#include "ServerManager.hpp"
#include <iostream>

int main(void)
{

	ServerManager server_manager;
	try
	{
		server_manager.runServer();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}