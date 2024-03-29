#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <sstream>
#include <string>
#include <regex>

#define RED "\e[91m"
#define CYAN "\033[36m"
#define RESET "\e[0m"

class ConfigData
{
public:
    ConfigData(std::string &input);
    ~ConfigData();

    void analyzeConfigData();
    void printConfigData();

private:
    std::string serverBlock;
    std::string serverPort;
    std::string serverHost;
    std::string serverName;

    void extractServerPort();
    void extractServerName();
};