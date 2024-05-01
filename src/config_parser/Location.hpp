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
    Location &operator=(const Location &other);
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
    std::string getSaveDir();
    void setLocationRoot(const std::string &root);
    void setLocationRoute(const std::string &route);

private:
    std::string locationBlock;
    std::string locationRoute;
    std::unordered_set<HttpMethod> acceptedMethods;
    std::string redirectionRoute;
    std::string root;
    std::string alias;
    bool directoryListing;
    std::string defaultFile;
    std::string saveDir;
    // std::string cgiExtension;
    // std::string cgiExecutor;
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
    void setSaveDir();
    // void setCgiExtension();
    // void setCgiExecutor();
};