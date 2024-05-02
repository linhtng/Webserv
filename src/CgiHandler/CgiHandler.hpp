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

    void createCgiProcess();
    void executeCgiScript();
    void readCgiOutput();
    void printEnv();
    std::string getCgiOutput();

private:
    void setupCgiEnv(const Request &request, const ConfigData &server);
    std::vector<const char *> createCgiEnvCharStr(std::vector<std::string> &cgiEnvStr);
    void closeCgiPipes();
    void closePipeEnd(int pipeFd);

    std::map<std::string, std::string> envMap;
    std::string cgiExecutorPathname;
    std::string cgiBinDir;
    int dataToCgiPipe[2] = {-1, -1};
    int dataFromCgiPipe[2] = {-1, -1};
    std::string cgiOutput;
};