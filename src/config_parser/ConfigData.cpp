#include "ConfigData.hpp"

ConfigData::ConfigData(std::string &input)
{
    serverBlock = input;
}

ConfigData::~ConfigData() {}

void ConfigData::analyzeConfigData()
{
    extractServerPort();
    extractServerName();
}

void ConfigData::printConfigData()
{
    std::cout << "Server name: " << serverName << std::endl;
    std::cout << "Server port: " << serverPort << std::endl;
    // std::cout << "Server host: " << serverHost << std::endl;
}

void ConfigData::extractServerPort()
{
    const std::string serverPortKey = "listen ";
    const std::string::size_type keyLength = serverPortKey.length();

    std::string::size_type start = serverBlock.find(serverPortKey);
    if (start == std::string::npos)
    {
        // Handle error: serverPortKey not found in serverBlock
        serverPort = 80;
        return;
    }
    std::string::size_type end = serverBlock.find(";", start);
    if (end == std::string::npos)
    {
        // Handle error: semicolon not found in serverBlock
        throw std::runtime_error("semicolon not found in serverBlock");
    }
    serverPort = serverBlock.substr(start + keyLength, end - start - keyLength);
}

void ConfigData::extractServerName()
{
    const std::string serverNameKey = "server_name ";
    const std::string::size_type keyLength = serverNameKey.length();

    std::string::size_type start = serverBlock.find(serverNameKey);
    if (start == std::string::npos)
    {
        serverName = "";
        return;
    }

    std::string::size_type end = serverBlock.find(";", start);
    if (end == std::string::npos)
    {
        // Handle error: semicolon not found in serverBlock
        throw std::runtime_error("semicolon not found in serverBlock");
    }

    serverName = serverBlock.substr(start + keyLength, end - start - keyLength);
}
