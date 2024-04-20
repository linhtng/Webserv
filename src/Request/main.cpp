#include "Request.hpp"
#include <iostream>
#include "../config_parser/ConfigData.hpp"
#include "Request.hpp"

int main()
{
	ConfigData config;
	Request request("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\nContent/Length: 10", config);
	std::cout << request.getContentLength() << std::endl;
	std::cout << request.bodyExpected() << std::endl;
	return 0;
}
