#include "Location.hpp"

Location::Location() {}

Location::Location(const std::string &input)
{
    locationBlock = input;
}

Location::~Location() {}

void Location::analyzeLocationData()
{
    setLocationRoute();
    // extractAcceptedMethods();
    // extractRedirection();
    // extractRoot();
    // extractDirectoryListing();
    // extractDefaultFile();
    // extractCgiExtension();
    // extractUploadPath();
}

void Location::printLocationData()
{
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