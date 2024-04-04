#include "Request.hpp"
#include <iostream>

int main()
{
	Request request("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\nContent/Length: 0");
	std::cout << request.getContentLength() << std::endl;
	return 0;
}
