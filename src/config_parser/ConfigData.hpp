#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <arpa/inet.h>
#include <map>
#include <regex>
#include <unordered_map>
#include "Location.hpp"
#include "../StringUtils/StringUtils.hpp"
#include "../FileSystemUtils/FileSystemUtils.hpp"
#include "../defines.hpp"

#define MAX_PORT 65535
#define MIN_PORT 1024
#define MAX_SERVER_NAME_LENGTH 253
#define MIN_ERROR_CODE 400
#define MAX_ERROR_CODE 599

namespace DirectiveKeys
{
	const std::string PORT = "listen";
	const std::string HOST = "host";
	const std::string SERVER_NAME = "server_name";
	const std::string ERROR_PAGE = "error_page";
	const std::string ClientBodySize = "client_max_body_size";
	const std::string CGI_DIR = "cgi_dir";
	const std::string CGI_EXTENSION = "cgi_exten";
	const std::string CGI_EXECUTOR = "cgi_executor";
	// Add more directive keys here
}

namespace DefaultValues
{
	const int PORT = 8080;
	const std::string HOST = "127.0.0.1";
	const std::string SERVER_NAME = "localhost";
	const long long MAX_CLIENT_BODY_SIZE = 1048576;
	const std::string CGI_DIR = "./cgi-bin";
}

class ConfigData
{
public:
	ConfigData();
	ConfigData(std::string &input);
	ConfigData(const ConfigData &other);
	ConfigData &operator=(const ConfigData &other);
	~ConfigData();

	void analyzeConfigData();
	void printConfigData();

	int getServerPort() const;
	std::string getServerPortString() const;
	std::string getServerName() const;
	std::string getServerHost() const;
	std::unordered_map<int, std::string> getErrorPages() const;
	size_t getMaxClientBodySize() const;
	std::map<std::string, Location> getLocations() const;
	std::string getCgiDir() const;
	std::vector<std::string> getCgiExtension() const;
	std::vector<std::string> getCgiExecutor() const;
	std::unordered_map<std::string, std::string> getCgiExtenExecutorMap() const;
	Location getMatchingLocation(std::string locationRoute) const;

private:
	std::string serverBlock;
	std::string serverPortString;
	int serverPort;
	std::string serverHost;
	std::string serverName;
	std::unordered_map<int, std::string> errorPages;
	std::string clientBodySize;
	size_t maxClientBodySize;
	std::vector<std::string> locationBlocks;
	std::map<std::string, Location> locations;
	std::string cgiDir;
	std::vector<std::string> cgiExtension;
	std::vector<std::string> cgiExecutor;
	std::unordered_map<std::string, std::string> cgiExtenExecutorMap;

	std::string extractDirectiveValue(const std::string &confBlock, const std::string &directiveKey);
	void extractMultipleArgValues(const std::string &directiveKey, std::vector<std::string> &values);
	void extractServerPort();
	bool validPortString(std::string &errorCodeStr);
	void extractServerName();
	void extractServerHost();
	void extractErrorPages();
	bool validErrorCode(std::string &errorCode);
	void extractMaxClientBodySize();
	void extractLocationBlocks();
	void extractCgiDir();
	void extractCgiExtension();
	void extractCgiExecutor();
	void extractcgiExtenExecutorMap();
	void splitLocationBlocks();
	void validateCgiExtension(std::string &extension);
};