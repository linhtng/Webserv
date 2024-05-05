#include "ConfigData.hpp"

ConfigData::ConfigData() {}

ConfigData::ConfigData(std::string &input)
{
	serverBlock = input;
	analyzeConfigData();
}

ConfigData::ConfigData(const ConfigData &other)
{
	*this = other;
}

ConfigData &ConfigData::operator=(const ConfigData &other)
{
	if (this != &other)
	{
		serverBlock = other.serverBlock;
		serverPort = other.serverPort;
		serverPortString = other.serverPortString;
		serverName = other.serverName;
		serverHost = other.serverHost;
		errorPages = other.errorPages;
		maxClientBodySize = other.maxClientBodySize;
		locationBlocks = other.locationBlocks;
		locations = other.locations;
		cgiDir = other.cgiDir;
		cgiExtension = other.cgiExtension;
		cgiExecutor = other.cgiExecutor;
	}
	return *this;
}

ConfigData::~ConfigData() {}

void ConfigData::analyzeConfigData()
{
	extractServerPort();
	extractServerName();
	extractServerHost();
	extractErrorPages();
	extractMaxClientBodySize();
	extractLocationBlocks();
	extractCgiDir();
	extractCgiExtension();
	extractCgiExecutor();
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
	std::cout << "Server port: " << serverPort << std::endl;
	std::cout << "Server host: " << serverHost << std::endl;
	std::cout << "Error pages: ";
	print(errorPages);
	std::cout << "Max client body size in bytes: " << maxClientBodySize << std::endl;
	std::cout << "CGI directory: " << cgiDir << std::endl;
	std::cout << "CGI extension: " << cgiExtension << std::endl;
	std::cout << "CGI executor: " << cgiExecutor << std::endl;
	// std::cout << "Locations: ";
	for (auto &location : locations)
	{
		location.second.printLocationData();
	}
}

std::string ConfigData::getServerHost() const
{
	return serverHost;
}

std::string ConfigData::extractDirectiveValue(const std::string &confBlock, const std::string &directiveKey)
{
	std::istringstream stream(confBlock);
	std::string line;
	int duplicate = 0;
	std::string returnValue = "";
	while (std::getline(stream, line))
	{
		std::regex directiveStartRegex("^\\s*" + directiveKey);
		if (std::regex_search(line, directiveStartRegex))
		{
			std::regex directiveRegex(directiveKey + "\\s+(\\S+)\\s*;");
			std::smatch match;
			if (std::regex_search(line, match, directiveRegex))
			{
				if (duplicate > 0)
				{
					throw std::runtime_error("Duplicate directive key: " + directiveKey);
				}
				returnValue = match[1].str();
				duplicate++;
			}
			else
				throw std::runtime_error("Invalid directive format: " + line);
		}
	}
	return returnValue;
}

/* Handling error:
- Invalid port number: not all characters in serverPortStr are digits
- Port number out of range: port number is not within the range 1024-65535
*/
bool ConfigData::validPortString(std::string &portStr)
{
	if (!std::all_of(portStr.begin(), portStr.end(), ::isdigit))
		return false;
	int portNumber = 0;
	try
	{
		portNumber = std::stoi(portStr);
	}
	catch (const std::out_of_range &)
	{
		throw std::runtime_error("Port out of range: " + portStr);
	}
	if (portNumber < MIN_PORT || portNumber > MAX_PORT)
	{
		throw std::runtime_error("Port out of range: " + portStr);
	}
	return true;
}

void ConfigData::extractServerPort()
{
	std::string serverPortStr = extractDirectiveValue(serverBlock, DirectiveKeys::PORT);
	if (serverPortStr.empty())
	{
		serverPort = DefaultValues::PORT;
		return;
	}
	if (!validPortString(serverPortStr))
	{
		throw std::runtime_error("Invalid port number: " + serverPortStr);
	}
	serverPortString = serverPortStr;
	serverPort = std::stoi(serverPortStr);
}

int ConfigData::getServerPort() const
{
	return serverPort;
}

std::string ConfigData::getServerPortString() const
{
	return serverPortString;
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
		std::cout << "Server name is empty. Using default server name: " << DefaultValues::SERVER_NAME << std::endl;
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
		serverHost = DefaultValues::HOST;
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
void ConfigData::extractErrorPages()
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
				errorPages[errorCode] = errorPage;
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

void ConfigData::extractCgiDir()
{
	std::string cgiDirStr = extractDirectiveValue(serverBlock, DirectiveKeys::CGI_DIR);
	if (cgiDirStr.empty())
	{
		cgiDirStr = DefaultValues::CGI_DIR;
	}
	if (!FileSystemUtils::pathExistsAndAccessible(cgiDirStr) || !FileSystemUtils::isDir(cgiDirStr))
		throw std::runtime_error("Invalid CGI directory: " + cgiDirStr);
	cgiDir = cgiDirStr;
}

void ConfigData::extractCgiExtension()
{
	cgiExtension = extractDirectiveValue(serverBlock, DirectiveKeys::CGI_EXTENSION);
}

void ConfigData::extractCgiExecutor()
{
	cgiExecutor = extractDirectiveValue(serverBlock, DirectiveKeys::CGI_EXECUTOR);
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
		size_t numberPartSizeT;
		try
		{
			numberPartSizeT = StringUtils::strToSizeT(numberPart);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("Invalid max client body size: " + maxClientBodySizeStr);
		}
		if (std::numeric_limits<size_t>::max() / multiplier < numberPartSizeT)
		{
			throw std::runtime_error("Out of range max client body size: " + maxClientBodySizeStr);
		}
		this->maxClientBodySize = numberPartSizeT * multiplier;
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

std::string ConfigData::getServerName() const
{
	return serverName;
}

std::unordered_map<int, std::string> ConfigData::getErrorPages() const
{
	return errorPages;
}

size_t ConfigData::getMaxClientBodySize() const
{
	return maxClientBodySize;
}

std::map<std::string, Location> ConfigData::getLocations() const
{
	return locations;
}

Location ConfigData::getMatchingLocation(std::string locationRoute) const
{
	std::vector<Location> matchingLocations;

	for (const auto &locationPair : this->locations)
	{
		const std::string &pathPattern = StringUtils::trimChar(locationPair.first, '/');
		std::cout << "Path pattern: " << pathPattern << std::endl;
		std::cout << "locationRoute.find(pathPattern)" << locationRoute.find(pathPattern) << std::endl;
		if (locationRoute.find(pathPattern) == 0)
		{
			matchingLocations.push_back(locationPair.second);
		}
	}
	std::cout << "Matching locations size: " << matchingLocations.size() << std::endl;
	if (matchingLocations.size() == 0)
	{
		throw std::runtime_error("No matching location found for route: " + locationRoute);
	}
	else
	{
		// Sort the matching locations by path length in descending order
		std::sort(matchingLocations.begin(), matchingLocations.end(), [](Location &a, Location &b)
				  { return a.getLocationRoute().size() > b.getLocationRoute().size(); });
		std::cout << "Matching location: " << matchingLocations[0].getLocationRoute() << std::endl;
		return matchingLocations[0];
	}
}

std::string ConfigData::getCgiDir() const
{
	return cgiDir;
}

std::string ConfigData::getCgiExtension() const
{
	return cgiExtension;
}

std::string ConfigData::getCgiExecutor() const
{
	return cgiExecutor;
}
