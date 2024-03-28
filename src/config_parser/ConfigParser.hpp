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

class ConfigData;

class ConfigParser
{
public:
    ConfigParser(std::string &fileName);
    ConfigParser(const ConfigParser &other);
    ConfigParser &operator=(const ConfigParser &other);
    ~ConfigParser();

    bool isValidFile(const std::string &filename);
    void readConfigFile(const std::string &fileName);
    void extractServerConfigs();

private:
    std::string fileContent;
    std::vector<std::string> configBlock;
    // std::vector<ConfigData> servers;
    int serverCount; // number of servers

    std::string removeComments(std::string &fullFileContent);
    void splitServerBlocks();
};