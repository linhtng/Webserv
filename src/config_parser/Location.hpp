#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <arpa/inet.h>
#include <map>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include "../defines.hpp"

class Location
{
public:
    Location();
    Location(const std::string &input);
    Location(const Location &location);
    ~Location();

    void analyzeLocationData();
    void printLocationData();

    /* Getters */
    std::string getLocationRoute();
    std::unordered_set<HttpMethod> getAcceptedMethods();
    std::string getRedirectionRoute();
    std::string getLocationRoot();
    std::string getLocationAlias();
    bool getDirectoryListing();
    std::string getDefaultFile();
    std::string getCgiExtension();
    std::string getCgiExecutor();

private:
    std::string locationBlock;
    std::string locationRoute;
    std::unordered_set<HttpMethod> acceptedMethods;
    std::string redirectionRoute;
    std::string root;
    std::string alias;
    bool directoryListing;
    std::string defaultFile;
    std::string cgiExtension;
    std::string cgiExecutor;
    // ... other properties ...

    std::string extractDirectiveValue(const std::string &directiveKey);
    void checkValidCharacters(const std::string &value, const std::regex &regexPattern);
    void setLocationRoute();
    void setAcceptedMethods();
    HttpMethod matchValidMethod(std::string method);
    void setRedirection();
    void setLocationRoot();
    void setLocationAlias();
    void setDirectoryListing();
    void setDefaultFile();
    void setCgiExtension();
    void setCgiExecutor();
};