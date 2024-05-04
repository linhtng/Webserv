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
	std::vector<ConfigData> configs;

	try
	{
		std::string fileName = "sample.conf";
		ConfigParser parser(fileName);
		parser.extractServerConfigs();
		// parser.printCluster(); // debug
		configs = parser.getServerConfigs();
	}
	catch (std::exception &e)
	{
		std::cerr << RED "[Invalid config file] " << e.what() << RESET << std::endl;
		return 1;
	}

	ConfigData config = configs[0];
	config.printConfigData();
	std::cout << std::endl;

	// Simple request
	// Request request(config, "GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 100");
	// request.printRequestProperties();

	// Request based on error code
	// Request request2(config, HttpStatusCode::IM_A_TEAPOT);
	// request2.printRequestProperties();
	// std::cout << std::endl;
	// Response response(request2);
	// printResponseBytes(response.formatResponse());

	// Request with redirect
	/*
	Request requestRedirect(configs, "GET /kapouet/pouic HTTP/1.0\r\nHost: localhost:8080\r\nConnection: close\r\n");
	requestRedirect.printRequestProperties();
	Response responseRedirect(requestRedirect);
	responseRedirect.printResponseProperties();
	std::cout << std::endl;
	printResponseBytes(responseRedirect.formatResponse());
	*/

	// request with multipart form
	std::cout << std::endl;
	/*
	Request requestMultipart(configs, "POST /hi HTTP/1.1\r\nHost: localhost:8081\r\nConnection: keep-alive\r\nContent-Length: 274\r\nContent-Type: multipart/form-data; boundary=----WebKitFormBoundaryohNq0PAELV5YFbkJ");


	std::string body = "------WebKitFormBoundaryohNq0PAELV5YFbkJ\r\nContent-Disposition: form-data; name=\"fileUpload\"; filename=\"test.sh\"\r\nContent-Type: text/x-sh\r\n\r\n#!/bin/bash\r\nfor i in {1..100}; do\r\n    curl -s -o /dev/null http://localhost:8080/ &\r\ndone\r\n------WebKitFormBoundaryohNq0PAELV5YFbkJ--";
	std::vector<std::byte> bodyBytes;
	for (char c : body)
	{
		bodyBytes.push_back(static_cast<std::byte>(c));
	}
	requestMultipart.appendToBody(bodyBytes);
	// requestMultipart.printRequestProperties();
	std::cout << std::endl;
	Response responseMulipart(requestMultipart);
	printResponseBytes(responseMulipart.formatResponse());
	*/

	// read smallFileUploadHeaders sample request from file
	std::string smallFileUploadHeaders = readFileIntoString("sample_requests/smallFileUploadHeaders");
	std::string smallFileUploadBody = readFileIntoString("sample_requests/smallFileUploadBody");
	std::vector<std::byte> smallFileUploadBodyBytes;
	for (char c : smallFileUploadBody)
	{
		smallFileUploadBodyBytes.push_back(static_cast<std::byte>(c));
	}
	Request requestMultipartSmall(configs, smallFileUploadHeaders);
	requestMultipartSmall.appendToBody(smallFileUploadBodyBytes);
	Response responseMultipartSmall(requestMultipartSmall);
	printResponseBytes(responseMultipartSmall.formatResponse());

	return 0;
}
