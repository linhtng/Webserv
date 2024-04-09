#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <sstream>
#include <string>
#include <arpa/inet.h>
#include <map>
#include <regex>
#include <unordered_map>

#define RED "\e[91m"
#define CYAN "\033[36m"
#define RESET "\e[0m"

class Location
{
public:
    Location();
    Location(const std::string &input);
    ~Location();

    void analyzeLocationData();
    void printLocationData();
    std::string getLocationRoute();

private:
    std::string locationBlock;
    std::string locationRoute;
    std::vector<std::string> acceptedMethods;
    std::string redirectionRoute;
    std::string root;
    std::string alias;
    bool directoryListing;
    std::string defaultFile;
    std::string cgiExtension;
    std::string uploadPath;
    // ... other properties ...

    void setLocationRoute();
    void setAcceptedMethods();
    void setRedirection();
    void setLocationRoot();
    void setLocationAlias();
    void setDirectoryListing();
    void setDefaultFile();
};