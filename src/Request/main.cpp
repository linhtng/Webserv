#include "Request.hpp"
#include <iostream>
#include "../config_parser/ConfigData.hpp"

int main()
{
	ConfigData config;
	Request request("GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nContent-Length: 100", config);
	request.printRequestProperties();
	Request request2(HttpStatusCode::BAD_GATEWAY, config);
	request2.printRequestProperties();
	return 0;
}
