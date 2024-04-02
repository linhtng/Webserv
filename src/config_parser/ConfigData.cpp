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
    extractServerHost();
}

void ConfigData::printConfigData()
{
    std::cout << "Server name: " << serverName << std::endl;
    std::cout << "Server port: " << serverPort << std::endl;
    std::cout << "Server host: " << serverHost << std::endl;
}

void trimSpace(std::string &str)
{
    std::regex pattern("^\\s+|\\s+$"); // Matches leading and trailing spaces
    str = std::regex_replace(str, pattern, "");
}

std::string ConfigData::extractDirectiveValue(const std::string &directiveKey)
{
    const std::string::size_type keyLength = directiveKey.length();

    std::string::size_type start = serverBlock.find(directiveKey);
    if (start == std::string::npos)
        return "";
    int keyValueBetween = serverBlock[start + keyLength];
    if (!std::isspace(keyValueBetween))
        throw std::runtime_error("Invalid directive: " + directiveKey);
    std::string::size_type end = serverBlock.find(";", start);
    if (end == std::string::npos)
        throw std::runtime_error("semicolon not found in serverBlock");
    std::string directiveValue = serverBlock.substr(start + keyLength, end - start - keyLength);
    trimSpace(directiveValue);
    return directiveValue;
}

/* Handling error:
- Invalid port number: not all characters in serverPortStr are digits
- Port number out of range: port number is not within the range 0-65535
*/
void ConfigData::extractServerPort()
{
    std::string serverPortStr = extractDirectiveValue(DirectiveKeys::PORT);
    if (serverPortStr.empty())
    {
        serverPort = DefaultValues::PORT;
        return;
    }
    if (!std::all_of(serverPortStr.begin(), serverPortStr.end(), ::isdigit))
    {
        throw std::runtime_error("Invalid port number: " + serverPortStr);
    }
    int port = 0;
    try
    {
        port = std::stoi(serverPortStr);
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Port number out of range: " + serverPortStr);
    }
    if (port < MIN_PORT || port > MAX_PORT)
        throw std::runtime_error("Port number out of range: " + serverPortStr);
    serverPort = port;
}

/* Handling error:
- Invalid server name: server name contains characters other than A-Z, a-z, 0-9, hyphen, and period
- Server name must not exceed 253 characters
*/
void ConfigData::extractServerName()
{
    std::string serverNameStr = extractDirectiveValue(DirectiveKeys::SERVER_NAME);
    if (serverNameStr.empty())
    {
        serverName = DefaultValues::SERVER_NAME;
        return;
    }
    // Regular expression for valid server names
    std::regex pattern("^[A-Za-z0-9-.]+$");
    if (std::regex_match(serverNameStr, pattern) && serverNameStr.size() <= 253)
    {
        serverName = serverNameStr;
    }
    else
    {
        throw std::runtime_error("Invalid server name: " + serverNameStr);
    }
}

void ConfigData::extractServerHost()
{
    std::string serverHostStr = extractDirectiveValue(DirectiveKeys::HOST);
    if (serverHostStr.empty() || serverHostStr == "localhost")
    {
        serverHost = "127.0.0.1";
        return;
    }
    // Check if serverHostStr is a valid IP address
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, serverHostStr.c_str(), &(sa.sin_addr));
    if (result == 1)
    {
        serverHost = serverHostStr;
        return;
    }
    else
    {
        throw std::runtime_error("Invalid server host: " + serverHostStr);
    }
}
