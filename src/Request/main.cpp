#include "Request.hpp"
#include "../Response/Response.hpp"
#include <iostream>
#include "../config_parser/ConfigData.hpp"
#include "../config_parser/ConfigParser.hpp"

std::string readFileIntoString(const std::string &filename)
{
	std::ifstream fileStream(filename);
	if (!fileStream)
	{
		std::cout << "Failed to open file: " << filename << std::endl;
		throw std::runtime_error("Failed to open file: " + filename);
	}

	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	return buffer.str();
}

void printResponseBytes(const std::vector<std::byte> &body)
{
	std::cout << std::endl;
	std::cout << "RESPONSE:" << std::endl;
	for (std::byte byte : body)
	{
		std::cout << static_cast<char>(byte);
	}
	std::cout << std::endl;
}

int main()
{
	std::vector<ConfigData> cluster;

	try
	{
		std::string fileName = "sample.conf";
		ConfigParser parser(fileName);
		parser.extractServerConfigs();
		// parser.printCluster(); // debug
		cluster = parser.getServerConfigs();
	}
	catch (std::exception &e)
	{
		std::cerr << RED "[Invalid config file] " << e.what() << RESET << std::endl;
		return 1;
	}

	ConfigData config = cluster[0];
	config.printConfigData();
	std::cout << std::endl;

	// Simple request
	Request request(config, "GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 100");
	// request.printRequestProperties();

	// Request based on error code
	Request request2(config, HttpStatusCode::INTERNAL_SERVER_ERROR);
	// request2.printRequestProperties();
	std::cout << std::endl;
	Response response(request2);
	printResponseBytes(response.formatResponse());

	// Request with redirect
	Request requestRedirect(config, "GET /hi/webservPage.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n");
	// requestRedirect.printRequestProperties();
	Response responseRedirect(requestRedirect);
	// responseRedirect.printResponseProperties();
	// printResponseBytes(responseRedirect.formatResponse());

	return 0;
}
