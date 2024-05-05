#pragma once
#include <unistd.h>
#include <string>
#include <iostream>
#include <map>
#include <signal.h>
#include "../Request/Request.hpp"
#include "../config_parser/ConfigParser.hpp"
#include "../defines.hpp"
#include "../StringUtils/StringUtils.hpp"
#include "../Logger/Logger.hpp"

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
    CgiHandler(const Request &request, std::unordered_map<std::string, std::string> &cgiParams);
    ~CgiHandler();

    void createCgiProcess();
    void printEnv();
    std::string getCgiOutput();
    int getCgiExitStatus();
    int execveStatus;

private:
    void setCgiExecutor(const ConfigData &server);
    void setupCgiEnv(const Request &request, const ConfigData &server, std::unordered_map<std::string, std::string> &cgiParams);
    std::vector<const char *> createCgiEnvCharStr(std::vector<std::string> &cgiEnvStr);
    void closeCgiPipes();
    void closePipeEnd(int pipeFd);
    void readCgiOutput();
    void executeCgiScript();
    void cgiTimeout(pid_t pid);

    std::map<std::string, std::string> envMap;
    std::string scriptName;
    std::string cgiExecutorPathname;
    std::string cgiBinDir;
    int dataToCgiPipe[2] = {-1, -1};
    int dataFromCgiPipe[2] = {-1, -1};
    std::string cgiOutput;
    std::vector<std::byte> messageBody;
    std::string messageBodyStr;
    HttpStatusCode cgiExitStatus;
};