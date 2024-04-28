#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <regex>
#include <unordered_set>

#include "ConfigData.hpp"

#define RED "\e[91m"
#define CYAN "\033[36m"
#define RESET "\e[0m"

class ConfigParser
{
public:
    ConfigParser(std::string &fileName);
    ~ConfigParser();

    void readConfigFile(const std::string &fileName);
    void extractServerConfigs();
    void printCluster();
    std::vector<ConfigData> getServerConfigs();

private:
    std::string fileContent;
    std::vector<std::string> configBlock;
    std::vector<ConfigData> servers;
    int serverCount; // number of servers

    std::string removeComments(std::string &fullFileContent);
    void splitServerBlocks();
};