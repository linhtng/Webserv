#include "Logger.hpp"

int main()
{
	Logger::initLogger();
	Logger::log(INFO, CLIENT, "%s disconnected", "mutsis");

	return 0;
}
