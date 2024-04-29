#pragma once
#include <unistd.h>
#include <string>
#include <iostream>
#include <map>
#include "../Request/Request.hpp"
#include "../config_parser/ConfigParser.hpp"
#include "../defines.hpp"

struct CgiRequest
{
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::map<std::string, std::string> headers;
};

class CgiHandler
{
public:
    CgiHandler();
    CgiHandler(const Request &request, const ConfigData &server);
    ~CgiHandler();

    void executeCgiScript();
    void sendCgiResponse();
    void printEnv();

private:
    void parseCgiRequest(const Request &request, const ConfigData &server);

    std::map<std::string, std::string> env;
    std::string cgiPath;
};