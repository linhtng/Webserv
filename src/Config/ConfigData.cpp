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
		cgiExtenExecutorMap = other.cgiExtenExecutorMap;
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
	extractcgiExtenExecutorMap();
}

// Generic print function
template <typename T>
void print(const T &container)
{
	for (const auto &elem : container)
	{
		std::cout << "elem first"
				  << " ";
		std::cout << elem.first << std::endl;
		std::cout << "elem second: " << elem.second << std::endl;
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
	std::cout << "cgiExtenExecutorMap: " << std::endl;
	print(cgiExtenExecutorMap);
	std::cout << "Locations: ";
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

void ConfigData::extractMultipleArgValues(const std::string &directiveKey, std::vector<std::string> &values)
{
	std::istringstream stream(serverBlock);
	std::string line;
	int duplicate = 0;
	while (std::getline(stream, line))
	{
		std::regex directiveLineRegex("^\\s*" + directiveKey);
		if (std::regex_search(line, directiveLineRegex))
		{
			std::regex validLineRegex(directiveKey + "\\s+(\\S+)\\s*(\\S+)?;");
			std::smatch match;
			if (std::regex_search(line, match, validLineRegex))
			{
				if (duplicate > 0)
				{
					throw std::runtime_error("Duplicate directive key: " + directiveKey);
				}
				values.push_back(match[1]);
				if (!match[2].str().empty())
					values.push_back(match[2]);
				duplicate++;
			}
			else
				throw std::runtime_error("Invalid directive in server block: " + line);
		}
	}
}

void ConfigData::extractcgiExtenExecutorMap()
{
	extractCgiExtension();
	extractCgiExecutor();
	if (cgiExtension.size() != cgiExecutor.size())
	{
		throw std::runtime_error("Number of CGI extensions and executors do not match");
	}
	cgiExtenExecutorMap.clear();
	for (size_t i = 0; i < cgiExtension.size(); i++)
	{
		cgiExtenExecutorMap[cgiExtension[i]] = cgiExecutor[i];
		// std::cout << "Extension: " << cgiExtension[i] << " Executor: " << cgiExecutor[i] << std::endl;
	}
	// Validate that extension matchs proper executor
	std::unordered_map<std::string, std::string> correctMap = {
		{".py", "python"},
		{".sh", "bash"}};
	for (const auto &pair : correctMap)
	{
		auto it = cgiExtenExecutorMap.find(pair.first);
		if (it != cgiExtenExecutorMap.end() && it->second.find(pair.second) == std::string::npos)
		{
			throw std::runtime_error("Invalid executor for extension " + pair.first + ": " + it->second);
		}
	}
}

void ConfigData::extractCgiExtension()
{
	extractMultipleArgValues(DirectiveKeys::CGI_EXTENSION, cgiExtension);
	for (auto &extension : cgiExtension)
	{
		validateCgiExtension(extension);
	}
}

void ConfigData::validateCgiExtension(std::string &extension)
{
	std::vector<std::string> validExtensions = VALID_CGI_EXTEN;
	auto itValid = std::find(validExtensions.begin(), validExtensions.end(), extension);
	if (itValid == validExtensions.end())
	{
		throw std::runtime_error("Invalid CGI extension:" + extension);
	}
}

void ConfigData::extractCgiExecutor()
{
	// cgiExecutor = extractDirectiveValue(serverBlock, DirectiveKeys::CGI_EXECUTOR);
	extractMultipleArgValues(DirectiveKeys::CGI_EXECUTOR, cgiExecutor);
	for (const auto &executor : cgiExecutor)
	{
		if (!FileSystemUtils::pathExistsAndAccessible(executor))
			throw std::runtime_error("Invalid CGI executor: " + executor);
	}
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

	std::cout << "getMatchingLocation(): Location route: " << locationRoute << std::endl;
	std::string trimmedLocationRoute = "/" + StringUtils::trimChar(locationRoute, '/');
	std::cout << "Trimmed location route: " << trimmedLocationRoute << std::endl;
	for (const auto &locationPair : this->locations)
	{
		std::string pathPattern = "/" + StringUtils::trimChar(locationPair.first, '/');
		std::cout << "Path pattern: " << pathPattern << std::endl;
		if (trimmedLocationRoute.find(pathPattern) == 0)
		{
			std::cout << "Found this location: " << locationPair.first << std::endl;
			// check if after the prefix lodaation string either ends or has a slash
			if (trimmedLocationRoute.size() == pathPattern.size() || trimmedLocationRoute[pathPattern.size()] == '/' || trimmedLocationRoute[pathPattern.size() - 1] == '/')
			{
				std::cout << "Matched location: " << locationPair.first << std::endl;
				matchingLocations.push_back(locationPair.second);
			}
			else
			{
				std::cout << "Didn't match this location after all because it's not dir: " << locationPair.first << std::endl;
			}
		}
	}
	std::cout << "Matching locations size: " << matchingLocations.size() << std::endl;
	if (matchingLocations.size() == 0)
	{
		throw std::runtime_error("No matching location found for trimmed route: " + trimmedLocationRoute);
	}
	else
	{
		// Sort the matching locations by path length in descending order
		std::sort(matchingLocations.begin(), matchingLocations.end(), [](Location &a, Location &b)
				  { return a.getLocationRoute().size() > b.getLocationRoute().size(); });
		std::cout << "Matching location: '" << matchingLocations[0].getLocationRoute() << "'" << std::endl;
		return matchingLocations[0];
	}
}

std::string ConfigData::getCgiDir() const
{
	return cgiDir;
}

std::vector<std::string> ConfigData::getCgiExtension() const
{
	return cgiExtension;
}

std::vector<std::string> ConfigData::getCgiExecutor() const
{
	return cgiExecutor;
}

std::unordered_map<std::string, std::string> ConfigData::getCgiExtenExecutorMap() const
{
	return cgiExtenExecutorMap;
}