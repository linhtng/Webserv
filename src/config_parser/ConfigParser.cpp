#include "ConfigParser.hpp"

ConfigParser::ConfigParser(std::string &fileName) : serverCount(0)
{
    this->readConfigFile(fileName);
}

ConfigParser::ConfigParser(const ConfigParser &other) : serverCount(other.serverCount) {}

ConfigParser &ConfigParser::operator=(const ConfigParser &other)
{
    if (this != &other)
    {
        serverCount = other.serverCount;
    }
    return *this;
}

ConfigParser::~ConfigParser() {}

bool ConfigParser::isValidFile(const std::string &filename)
{
    return std::filesystem::is_regular_file(filename);
}

void ConfigParser::readConfigFile(const std::string &fileName)
{
    std::ifstream inputFile(fileName);
    if (!inputFile || !isValidFile(fileName))
    {
        throw std::runtime_error("Error: unable to open input file " + fileName);
    }
    std::string fullFileContent = std::string((std::istreambuf_iterator<char>(inputFile)),
                                              (std::istreambuf_iterator<char>()));
    fileContent = removeComments(fullFileContent);
    // std::cout << "File content: " << this->fileContent << std::endl;
}

/*
1. Split the content into separate server blocks
2. Remove comments from each block
3. Extract server name, number, etc. from each block
4. Save each server's information in a ServerConfig struct
5. Return a vector of ServerConfig structs */

void ConfigParser::extractServerConfigs()
{
    // std::cout << "Extracting server configurations...\n";
    splitServerBlocks();
    for (int i = 0; i < serverCount; i++)
    {
        ConfigData serverConfig(configBlock[i]);
        serverConfig.analyzeConfigData();
        servers.push_back(serverConfig);
    }
    for (auto &server : servers)
    {
        server.printConfigData();
    }
}

std::string ConfigParser::removeComments(std::string &fullFileContent)
{
    std::regex commentRegex("#.*"); // Matches any line starting with '#'
    return std::regex_replace(fullFileContent, commentRegex, "");
    // std::cout << "File content after removing comments: " << fileContent << std::endl;
}

void ConfigParser::splitServerBlocks()
{
    std::istringstream iss(fileContent);
    std::string line;
    std::string currentBlock;
    int braceCount = 0;
    bool insideServerBlock = false;

    while (std::getline(iss, line))
    {
        if (line.find("server {") != std::string::npos)
            insideServerBlock = true;
        if (insideServerBlock)
        {
            if (line.find("{") != std::string::npos)
                braceCount++;
            currentBlock += line + "\n";
            if (line.find("}") != std::string::npos)
            {
                braceCount--;
                if (braceCount == 0)
                {
                    configBlock.push_back(currentBlock);
                    currentBlock.clear();
                    serverCount++;
                    insideServerBlock = false;
                }
            }
        }
    }
    // for (auto &block : configBlock)
    // {
    //     std::cout << "Block: " << std::endl;
    //     std::cout << block << std::endl;
    // }
    // std::cout << "Number of servers: " << serverCount << std::endl;
}
