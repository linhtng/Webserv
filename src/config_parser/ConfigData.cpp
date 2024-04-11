#include "ConfigData.hpp"

ConfigData::ConfigData(std::string &input)
{
    serverBlock = input;
}

ConfigData::~ConfigData() {}

void ConfigData::analyzeConfigData()
{
    extractServerPorts();
    extractServerName();
    extractServerHost();
    extractDefaultErrorPages();
    extractMaxClientBodySize();
    extractLocationBlocks();
}

// Generic print function
template <typename T>
void print(const T &container)
{
    for (const auto &elem : container)
    {
        std::cout << "elem: "
                  << " ";
        std::cout << elem.first << " " << elem.second << std::endl;
    }
}

template <typename T>
void printVector(const std::vector<T> &vec)
{
    for (const T &element : vec)
    {
        // std::cout << "Elem: ";
        std::cout << element << " ";
    }
    std::cout << "\n";
}

void ConfigData::printConfigData()
{
    std::cout << "Server name: " << serverName << std::endl;
    std::cout << "Server port: ";
    printVector(serverPorts);
    std::cout << "Server host: " << serverHost << std::endl;
    std::cout << "Error pages: ";
    print(defaultErrorPages);
    std::cout << "Max client body size in bytes: " << maxClientBodySize << std::endl;
    // std::cout << "Location blocks: ";
    // printVector(locationBlocks);
    // std::cout << "Locations: ";
    for (auto &location : locations)
    {
        location.second.printLocationData();
    }
}

void trimSpace(std::string &str)
{
    std::regex pattern("^\\s+|\\s+$"); // Matches leading and trailing spaces
    str = std::regex_replace(str, pattern, "");
}

std::string ConfigData::extractDirectiveValue(const std::string confBlock, const std::string &directiveKey)
{
    const std::string::size_type keyLength = directiveKey.length();

    std::string::size_type start = confBlock.find(directiveKey);
    if (start == std::string::npos)
        return "";
    int keyValueBetween = confBlock[start + keyLength];
    if (!std::isspace(keyValueBetween))
        throw std::runtime_error("Invalid directive: " + directiveKey);
    std::string::size_type end = confBlock.find(";", start);
    if (end == std::string::npos)
        throw std::runtime_error("semicolon not found in serverBlock");
    std::string directiveValue = confBlock.substr(start + keyLength, end - start - keyLength);
    trimSpace(directiveValue);
    return directiveValue;
}

/* Handling error:
- Invalid port number: not all characters in serverPortStr are digits
- Port number out of range: port number is not within the range 1024-65535
*/
bool ConfigData::validPortString(std::string &portStr)
{
    if (!std::all_of(portStr.begin(), portStr.end(), ::isdigit))
        return false;
    int errorCode = 0;
    try
    {
        errorCode = std::stoi(portStr);
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Port out of range: " + portStr);
    }
    if (errorCode < MIN_PORT || errorCode > MAX_PORT)
    {
        throw std::runtime_error("Port out of range: " + portStr);
    }
    return true;
}

void ConfigData::extractServerPorts()
{
    std::istringstream stream(serverBlock);
    std::string line;
    while (std::getline(stream, line))
    {
        std::regex listenRegex("^\\s*listen");
        if (std::regex_search(line, listenRegex))
        {
            std::regex serverPortRegex("listen\\s+(\\S+)\\;");
            std::smatch match;
            if (std::regex_search(line, match, serverPortRegex))
            {
                std::string portString(match[1]);
                if (portString.empty() || !validPortString(portString))
                {
                    throw std::invalid_argument("Invalid listen directive in server block " + line);
                }
                int portNumber = std::stoi(portString);
                if (std::find(serverPorts.begin(), serverPorts.end(), portNumber) != serverPorts.end())
                {
                    throw std::invalid_argument("Duplicate port number: " + portString);
                }
                serverPorts.push_back(portNumber);
            }
            else
                throw std::invalid_argument("Invalid listen directive in server block " + line);
        }
    }
    if (serverPorts.empty())
    {
        serverPorts.push_back(DefaultValues::PORT);
    }
}

/* Handling error:
- Invalid server name: server name contains characters other than A-Z, a-z, 0-9, hyphen, and period
- Server name must not exceed 253 characters
*/
void ConfigData::extractServerName()
{
    std::string serverNameStr = extractDirectiveValue(serverBlock, DirectiveKeys::SERVER_NAME);
    if (serverNameStr.empty())
    {
        serverName = DefaultValues::SERVER_NAME;
        return;
    }
    // Regular expression for valid server names
    std::regex pattern("^[A-Za-z0-9-.]+$");
    if (std::regex_match(serverNameStr, pattern) && serverNameStr.size() <= MAX_SERVER_NAME_LENGTH)
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
    std::string serverHostStr = extractDirectiveValue(serverBlock, DirectiveKeys::HOST);
    if (serverHostStr.empty() || serverHostStr == "localhost")
    {
        serverHost = "127.0.0.1";
        return;
    }
    // Check if serverHostStr is a valid IP address
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, serverHostStr.c_str(), &(sa.sin_addr));
    if (result != 1)
        throw std::runtime_error("Invalid server host: " + serverHostStr);
    serverHost = serverHostStr;
}

/* Validating error codes only.
No error page URIs validation is done here.
The line must be in format error_page <error_code> <error_page_uri>;
If not in this format, i.e. 0 or more than one error page URI is provided in one line, throw invalid number of arguments error.
*/
void ConfigData::extractDefaultErrorPages()
{
    std::istringstream stream(serverBlock);
    std::string line;
    while (std::getline(stream, line))
    {
        std::regex errorPageRegex("^\\s*error_page");
        if (std::regex_search(line, errorPageRegex))
        {
            std::regex validErrorPageRegex("error_page\\s+(\\S+)(\\s+(\\S+))?;");
            std::smatch match;
            if (std::regex_search(line, match, validErrorPageRegex))
            {
                if (match[2].str().empty())
                {
                    throw std::runtime_error("Invalid number of arguments: " + line);
                }
                std::string errorCodeStr(match[1]);
                if (!validErrorCode(errorCodeStr))
                {
                    throw std::runtime_error("Invalid error page directive in server block: " + line);
                }
                int errorCode = std::stoi(errorCodeStr);
                std::string errorPage = match[2];
                defaultErrorPages[errorCode] = errorPage;
            }
            else
                throw std::runtime_error("Invalid error page directive in server block: " + line);
        }
    }
}

/* Handling error for error codes parameter in error_page directive:
- Error code is not a number
- Error code out of range: error code is not within the range 400-599
*/
bool ConfigData::validErrorCode(std::string &errorCodeStr)
{
    if (!std::all_of(errorCodeStr.begin(), errorCodeStr.end(), ::isdigit))
        return false;
    int errorCode = 0;
    try
    {
        errorCode = std::stoi(errorCodeStr);
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Error code out of range: " + errorCodeStr);
    }
    if (errorCode < MIN_ERROR_CODE || errorCode > MAX_ERROR_CODE)
    {
        throw std::runtime_error("Error code out of range: " + errorCodeStr);
    }
    return true;
}

/* In nginx, setting size to 0 means no limit on client body size.
But we don't allow that. 0 is invalid.
*/
void ConfigData::extractMaxClientBodySize()
{
    std::string maxClientBodySizeStr = extractDirectiveValue(serverBlock, DirectiveKeys::ClientBodySize);
    if (maxClientBodySizeStr.empty())
    {
        maxClientBodySize = DefaultValues::MAX_CLIENT_BODY_SIZE;
        return;
    }
    std::unordered_map<std::string, int> units = {
        {"k", 1024},
        {"K", 1024},
        {"m", 1024 * 1024},
        {"M", 1024 * 1024},
        {"g", 1024 * 1024 * 1024},
        {"G", 1024 * 1024 * 1024}};

    std::regex pattern("(\\d+)([kKmMgG]?)");
    std::smatch matches;
    if (std::regex_match(maxClientBodySizeStr, matches, pattern))
    {
        std::string numberPart = matches[1].str();
        std::string unit = matches[2].str();
        int multiplier = 1;
        if (units.find(unit) != units.end())
        {
            multiplier = units[unit];
        }
        maxClientBodySize = std::stoll(numberPart) * multiplier;
        if (maxClientBodySize <= 0 || maxClientBodySize > INT_MAX)
        {
            throw std::runtime_error("Out of range max client body size: " + maxClientBodySizeStr);
        }
    }
    else
    {
        throw std::runtime_error("Invalid max client body size: " + maxClientBodySizeStr);
    }
}

/* Extract location blocks from server block and create Location objects for each location block. If a location block has a route that already exists in the locations map, skip it.
 */
void ConfigData::extractLocationBlocks()
{
    splitLocationBlocks();
    for (const auto &locationBlock : locationBlocks)
    {
        Location location(locationBlock);
        location.analyzeLocationData();
        std::string route = location.getLocationRoute();
        if (locations.find(route) == locations.end())
            locations[location.getLocationRoute()] = location;
    }
}

void ConfigData::splitLocationBlocks()
{
    std::istringstream iss(serverBlock);
    std::string line;
    std::string currentBlock;
    int braceCount = 0;
    bool insideBlock = false;

    while (std::getline(iss, line))
    {
        if (line.find("location") != std::string::npos && line.find(" {") != std::string::npos)
            insideBlock = true;
        if (insideBlock)
        {
            if (line.find("{") != std::string::npos)
                braceCount++;
            currentBlock += line + "\n";
            if (line.find("}") != std::string::npos)
            {
                braceCount--;
                if (braceCount == 0)
                {
                    locationBlocks.push_back(currentBlock);
                    currentBlock.clear();
                    insideBlock = false;
                }
            }
        }
    }
}

std::vector<int> ConfigData::getServerPorts() const
{
    return serverPorts;
}

std::string ConfigData::getServerName() const
{
    return serverName;
}