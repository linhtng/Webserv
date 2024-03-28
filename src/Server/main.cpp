#include "Server.hpp"
#include <iostream>

int main (void)
{

	Server server;
	try {
		server.runServer();
	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}