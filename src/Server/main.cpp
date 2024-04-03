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
		server_manager.handleException(e);
		return 1;
	}

	return 0;
}