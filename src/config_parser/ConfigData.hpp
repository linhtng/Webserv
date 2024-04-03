#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <sstream>
#include <string>
#include <arpa/inet.h>
#include <map>
#include <regex>
#include <unordered_map>

#define RED "\e[91m"
#define CYAN "\033[36m"
#define RESET "\e[0m"

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
    // Add more directive keys here
}

namespace DefaultValues
{
    const int PORT = 8080;
    const std::string HOST = "127.0.0.1";
    const std::string SERVER_NAME = "localhost";
}

class ConfigData
{
public:
    ConfigData(std::string &input);
    ~ConfigData();

    void analyzeConfigData();
    void printConfigData();

private:
    std::string serverBlock;
    int serverPort;
    std::string serverHost;
    std::string serverName;
    std::vector<std::string> errorPagesValues;
    std::unordered_map<int, std::string> defaultErrorPages;

    std::string extractDirectiveValue(const std::string confBlock, const std::string &directiveKey);
    void extractServerPort();
    void extractServerName();
    void extractServerHost();
    void extractDefaultErrorPages();
};