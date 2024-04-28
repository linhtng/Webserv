#pragma once
#include <unistd.h>
#include <string>
#include <iostream>
#include <map>
#include "../Request/Request.hpp"
#include "../config_parser/ConfigParser.hpp"

#define GATEWAY_INTERFACE "CGI/1.1"

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
    CgiHandler(const Request &request, Location &cgiBin);
    ~CgiHandler();

    void executeCgiScript();
    void sendCgiResponse();

private:
    void parseCgiRequest(const Request &request, Location &cgiBin);
    std::string setPathInfo(const Request &request, Location &cgiBin);

    std::map<std::string, std::string> env;
    std::string cgiPath;
};