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
    ConfigData &operator=(const ConfigData &other);
    ~ConfigData();

    void analyzeConfigData();
    void printConfigData();

    int getServerPort() const;
    std::string getServerName() const;
    std::string getServerHost() const;
    std::unordered_map<int, std::string> getDefaultErrorPages() const;
    size_t getMaxClientBodySize() const;
    std::map<std::string, Location> getLocations() const;
    std::string getCgiDir() const;
    std::string getCgiExtension() const;
    std::string getCgiExecutor() const;
    bool hasMatchingLocation(std::string locationRoute) const;
    Location getMatchingLocation(std::string locationRoute) const;

private:
    std::string serverBlock;
    // std::vector<int> serverPorts;
    int serverPort;
    std::string serverHost;
    std::string serverName;
    std::unordered_map<int, std::string> defaultErrorPages;
    std::string clientBodySize;
    size_t maxClientBodySize;
    std::vector<std::string> locationBlocks;
    std::map<std::string, Location> locations;
    std::string cgiDir;
    std::string cgiExtension;
    std::string cgiExecutor;

    std::string extractDirectiveValue(const std::string &confBlock, const std::string &directiveKey);
    void extractServerPort();
    bool validPortString(std::string &errorCodeStr);
    void extractServerName();
    void extractServerHost();
    void extractDefaultErrorPages();
    bool validErrorCode(std::string &errorCode);
    void extractMaxClientBodySize();
    void extractLocationBlocks();
    void extractCgiDir();
    void extractCgiExtension();
    void extractCgiExecutor();
    void splitLocationBlocks();
};