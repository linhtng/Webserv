#include "Location.hpp"

Location::Location() {}

Location::Location(const std::string &input)
{
    locationBlock = input;
    root = "";
    alias = "";
    directoryListing = false;
    defaultFile = "index.html";
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
    // if (alias.empty())
    //     setLocationRoot();
    if (root.empty() && alias.empty())
    {
        throw std::runtime_error("Root or alias must be set in location block: " + locationBlock);
    }
    setDirectoryListing();
    setDefaultFile();
    setCgiExtension();
    setCgiPath();
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
    std::cout << "Cgi extension: " << cgiExtension << std::endl;
    std::cout << "Cgi path: " << cgiPath << std::endl;
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
}

/* Regex pattern: limit_except\s+((\S+\s*)+)\{
 * This pattern matches the limit_except block and extracts the accepted methods
 * Example: limit_except GET POST {
 * The accepted methods are GET and POST
 * Note that we are not validating what is in the {} after the methods
 */
void Location::setAcceptedMethods()
{
    std::regex methodRegex("limit_except\\s+(([A-Z]+\\s*)+)\\{");
    std::smatch match;
    if (std::regex_search(locationBlock, match, methodRegex))
    {
        std::string methods = match[1].str();
        std::istringstream iss(methods);
        std::string method;
        while (iss >> method)
        {
            acceptedMethods.push_back(method);
        }
    }
}

void Location::setRedirection()
{
    std::regex redirectionRegex("return\\s+(\\S+\\s*)\\;");
    std::smatch match;
    if (std::regex_search(locationBlock, match, redirectionRegex))
    {
        redirectionRoute = match[1].str();
    }
}

void Location::setLocationRoot()
{
    std::regex rootRegex("root\\s+(\\S+\\s*)\\;");
    std::smatch match;
    if (std::regex_search(locationBlock, match, rootRegex))
    {
        root = match[1].str();
    }
}

void Location::setLocationAlias()
{
    std::regex aliasRegex("alias\\s+(\\S+\\s*)\\;");
    std::smatch match;
    if (std::regex_search(locationBlock, match, aliasRegex))
    {
        alias = match[1].str();
    }
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

void Location::setDefaultFile()
{
    std::regex defaultFileRegex("\\bindex\\s+(\\S*\\s*)?\\;"); // work for normal case and detect if no argument ok. Not work for case with multiple arguments

    std::smatch match;
    if (std::regex_search(locationBlock, match, defaultFileRegex))
    {
        if (match[1].length() == 0)
        {
            throw std::invalid_argument("Missing argument for index directive in route " + locationRoute);
        }
        defaultFile = match[1].str();
        // Check for invalid characters
        std::regex invalidCharRegex("[\\\\/;:*?<>|]");
        if (std::regex_search(defaultFile, invalidCharRegex))
        {
            throw std::invalid_argument("Invalid character in default file name " + defaultFile);
        }
    }
}

void Location::setCgiExtension()
{
    std::regex cgiExtensionRegex("\\bcgi-exten\\s+(\\S+\\s*)?\\;");
    std::smatch match;
    if (std::regex_search(locationBlock, match, cgiExtensionRegex))
    {
        if (match[1].length() == 0)
        {
            throw std::invalid_argument("Missing argument for cgi-exten directive in route " + locationRoute);
        }
        cgiExtension = match[1].str();
        // Check for invalid characters
        std::regex invalidCharRegex("[\\\\/;:*?<>|]");
        if (std::regex_search(cgiExtension, invalidCharRegex))
        {
            throw std::invalid_argument("Invalid character for cgi-exten " + cgiExtension);
        }
    }
}

void Location::setCgiPath()
{
    std::regex cgiPathRegex("\\bcgi-path\\s+(\\S+\\s*)?\\;");
    std::smatch match;
    if (std::regex_search(locationBlock, match, cgiPathRegex))
    {
        if (match[1].length() == 0)
        {
            throw std::invalid_argument("Missing argument for cgi-exten directive in route " + locationRoute);
        }
        cgiPath = match[1].str();
        // Check for invalid characters
        std::regex invalidCharRegex("[;:*?<>|]");
        if (std::regex_search(cgiPath, invalidCharRegex))
        {
            throw std::invalid_argument("Invalid character for cgi-exten " + cgiPath);
        }
    }
}
