#include "Request.hpp"
#include "../Response/Response.hpp"
#include <iostream>
#include "../config_parser/ConfigData.hpp"

std::string readFileIntoString(const std::string &filename)
{
	std::ifstream fileStream(filename);
	if (!fileStream)
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	return buffer.str();
}

int main()
{
	std::string fileContent = readFileIntoString("sample.conf");
	ConfigData config(fileContent);
	Request request(config, "GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 100");
	request.printRequestProperties();
	std::cout << std::endl;
	Request request2(config, HttpStatusCode::INTERNAL_SERVER_ERROR);
	request2.printRequestProperties();
	std::cout << std::endl;
	Response response(request2);
	std::vector<std::byte> responseBytes = response.formatResponse();
	for (std::byte byte : responseBytes)
	{
		std::cout << static_cast<char>(byte);
	}
	return 0;
}
