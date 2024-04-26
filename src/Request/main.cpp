#include "Request.hpp"
#include "../Response/Response.hpp"
#include <iostream>
#include "../config_parser/ConfigData.hpp"

int main()
{
	ConfigData config;
	Request request(config, "GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nContent-Length: 100");
	request.printRequestProperties();
	std::cout << std::endl;
	Request request2(config, HttpStatusCode::BAD_GATEWAY);
	request2.printRequestProperties();
	Response response(request2);
	return 0;
}
