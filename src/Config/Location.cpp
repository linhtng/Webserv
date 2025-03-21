#include "Location.hpp"

Location::Location() {}

Location::Location(const std::string &input)
{
	locationBlock = input;
	root = "";
	alias = "";
	directoryListing = false;
	defaultFile = "index.html";
	redirectionRoute = "";
	saveDirIsEmpty = true;
	aliasIsEmpty = true;
	rootIsEmpty = true;
	redirectionIsEmpty = true;
}

Location &Location::operator=(const Location &other)
{
	if (this == &other)
	{
		return *this;
	}
	locationBlock = other.locationBlock;
	locationRoute = other.locationRoute;
	acceptedMethods = other.acceptedMethods;
	redirectionRoute = other.redirectionRoute;
	root = other.root;
	alias = other.alias;
	directoryListing = other.directoryListing;
	defaultFile = other.defaultFile;
	saveDir = other.saveDir;
	saveDirIsEmpty = other.saveDirIsEmpty;
	aliasIsEmpty = other.aliasIsEmpty;
	rootIsEmpty = other.rootIsEmpty;
	redirectionIsEmpty = other.redirectionIsEmpty;
	return *this;
}

Location::~Location() {}

/* Best to avoid having both root and alias set in the same location block.
 * If both are set, alias will be used.
 TODO: https://nginx.org/en/docs/http/ngx_http_core_module.html#alias:
When location matches the last part of the directive’s value:

location /images/ {
	alias /data/w3/images/;
}
it is better to use the root directive instead:

location /images/ {
	root /data/w3;
}
 */
void Location::analyzeLocationData()
{
	setLocationRoute();
	setAcceptedMethods();
	setRedirection();
	setLocationAlias();
	setLocationRoot();
	if (rootIsEmpty && aliasIsEmpty)
	{
		throw std::runtime_error("Root or alias must be set in location block: " + locationBlock);
	}
	/* if (!root.empty() && !alias.empty())
	{
		throw std::runtime_error("Don't have both root and alias set in location block: " + locationBlock);
	} */
	setDirectoryListing();
	setDefaultFile();
	setSaveDir();
	// setCgiExtension();
	// setCgiExecutor();
}

void Location::printLocationData()
{
	// std::cout << "Location block: " << locationBlock << std::endl;
	std::cout << "Location route: " << locationRoute << std::endl;
	std::cout << "Accepted methods: ";
	for (const auto &method : acceptedMethods)
	{
		std::cout << method << " ";
	}
	std::cout << std::endl;
	std::cout << "Redirection rout: " << redirectionRoute << std::endl;
	std::cout << "Root: " << root << std::endl;
	std::cout << "Alias: " << alias << std::endl;
	std::cout << "Directory listing: " << (directoryListing ? "on" : "off") << std::endl;
	std::cout << "Default file: " << defaultFile << std::endl;
	std::cout << "Save dir: " << saveDir << std::endl;
	std::cout << std::endl;
}

std::string Location::getLocationRoute()
{
	return locationRoute;
}

/* Regex pattern: location\s+([^\\s]+)\s*\{
 * This pattern matches the location block and extracts the route
 * Example: location /api {
 * The route is /api
 */
void Location::setLocationRoute()
{
	std::regex locationRegex("location\\s+([^\\s]+)\\s*\\{");
	std::smatch match;
	if (std::regex_search(locationBlock, match, locationRegex))
	{
		locationRoute = match[1].str();
	}
	else
	{
		throw std::runtime_error("Invalid location block: " + locationBlock);
	}
	locationRoute = StringUtils::trimChar(locationRoute, '/');
}

/* Regex pattern: limit_except\s+((\S+\s*)+)\{
 * This pattern matches the limit_except block and extracts the accepted methods
 * Example: limit_except GET POST {
 * The accepted methods are GET and POST
 * Note that we are not validating what is in the {} after the methods
 */
void Location::setAcceptedMethods()
{
	std::regex methodRegex("allowed_method\\s+(([A-Z]+\\s*)+)\\;");
	std::smatch match;
	if (std::regex_search(locationBlock, match, methodRegex))
	{
		std::string methods = match[1].str();
		std::istringstream iss(methods);
		std::string method;
		while (iss >> method)
		{
			acceptedMethods.insert(matchValidMethod(method));
		}
	}
	else if (locationBlock.find("allowed_method") != std::string::npos)
	{
		std::cout << "Invalid allowed_method syntax: " << locationBlock << std::endl;
		throw std::runtime_error("Invalid allowed_method syntax");
	}
}

HttpMethod Location::matchValidMethod(std::string method)
{
	auto it = HttpUtils::_strToHttpMethod.find(method);
	if (it == HttpUtils::_strToHttpMethod.end())
	{
		throw std::invalid_argument("Invalid method: " + method);
	}
	return it->second;
}

void Location::setRedirection()
{
	// std::regex redirectionRegex("return\\s+(\\S+\\s*)\\;");
	// std::smatch match;
	// if (std::regex_search(locationBlock, match, redirectionRegex))
	// {
	//     redirectionRoute = match[1].str();
	// }
	redirectionRoute = extractDirectiveValue("return");
	if (!redirectionRoute.empty())
	{
		redirectionIsEmpty = false;
	}
	redirectionRoute = StringUtils::trimChar(redirectionRoute, '/');
}

void Location::setLocationRoot()
{
	std::regex rootRegex("root\\s+(\\S+\\s*)\\;");
	std::smatch match;
	if (std::regex_search(locationBlock, match, rootRegex))
	{
		root = match[1].str();
	}
	if (!root.empty())
	{
		rootIsEmpty = false;
	}
	root = StringUtils::trimChar(root, '/');
}

void Location::setLocationAlias()
{
	std::regex aliasRegex("alias\\s+(\\S+\\s*)\\;");
	std::smatch match;
	if (std::regex_search(locationBlock, match, aliasRegex))
	{
		alias = match[1].str();
	}
	if (!alias.empty())
	{
		aliasIsEmpty = false;
	}
	alias = StringUtils::trimChar(alias, '/');
}

void Location::setDirectoryListing()
{
	std::regex directoryListingRegex("autoindex\\s+(on|off)\\;");
	std::smatch match;
	if (std::regex_search(locationBlock, match, directoryListingRegex))
	{
		std::string value = match[1].str();
		directoryListing = value == "on";
	}
}

std::string Location::extractDirectiveValue(const std::string &directiveKey)
{
	std::istringstream stream(locationBlock);
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

void Location::checkValidCharacters(const std::string &value, const std::regex &regexPattern)
{
	if (std::regex_search(value, regexPattern))
	{
		throw std::invalid_argument("Invalid character in value " + value);
	}
}

void Location::setDefaultFile()
{
	std::string defaultFileValue = extractDirectiveValue("index");
	if (!defaultFileValue.empty())
	{
		std::regex invalidCharRegex("[\\\\/;:*?<>|]");
		checkValidCharacters(defaultFileValue, invalidCharRegex);
		defaultFile.clear();
		defaultFile = defaultFileValue;
	}
}

void Location::setSaveDir()
{
	saveDir = extractDirectiveValue("save_dir");
	if (!saveDir.empty())
	{
		saveDirIsEmpty = false;
	}
	saveDir = StringUtils::trimChar(saveDir, '/');
}

// void Location::setCgiExtension()
// {
//     std::string cgiExtenValue = extractDirectiveValue("cgi_exten");
//     if (!cgiExtenValue.empty())
//     {
//         std::regex invalidCharRegex("[\\\\/;:*?<>|]");
//         checkValidCharacters(cgiExtenValue, invalidCharRegex);
//         cgiExtension = cgiExtenValue;
//     }
// }

// void Location::setCgiExecutor()
// {
//     cgiExecutor = extractDirectiveValue("cgi_executor");
// }

/* GETTERS */
std::unordered_set<HttpMethod> Location::getAcceptedMethods()
{
	return acceptedMethods;
}

std::string Location::getRedirectionRoute()
{
	return redirectionRoute;
}

std::string Location::getLocationRoot()
{
	return root;
}

std::string Location::getLocationAlias()
{
	return alias;
}

bool Location::getDirectoryListing()
{
	return directoryListing;
}

std::string Location::getDefaultFile()
{
	return defaultFile;
}

std::string Location::getSaveDir()
{
	return saveDir;
}

void Location::setLocationRoot(const std::string &root)
{
	this->root = root;
}

void Location::setLocationRoute(const std::string &route)
{
	locationRoute = route;
}

bool Location::getSaveDirIsEmpty()
{
	return saveDirIsEmpty;
}

bool Location::getAliasIsEmpty()
{
	return aliasIsEmpty;
}

bool Location::getRootIsEmpty()
{
	return rootIsEmpty;
}

bool Location::getRedirectionIsEmpty()
{
	return redirectionIsEmpty;
}