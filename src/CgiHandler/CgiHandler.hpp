#pragma once
#include <string>
#include <iostream>
#include <map>
#include "../Request/Request.hpp"
#include "../config_parser/ConfigParser.hpp"

#define CGI_PATH "cgi-bin"

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
    CgiHandler(const std::string &path, const Request &request);
    ~CgiHandler();

    void executeCgiScript();
    void sendCgiResponse();

private:
    void parseCgiRequest(const Request &request);

    std::map<std::string, std::string> env;
    std::string cgiPath;
};