#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const Request &request, const ConfigData &server)
{
    setupCgiEnv(request, server);
}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::setupCgiEnv(const Request &request, const ConfigData &server)
{
    std::string target = request.getTarget();
    envMap["REQUEST_METHOD"] = request.getMethodStr();
    envMap["CONTENT_TYPE"] = request.getContentType();
    envMap["CONTENT_LENGTH"] = std::to_string(request.getContentLength());
    // envMap["QUERY_STRING"] = request.getQuery();
    // PATH_INFO = test.py
    envMap["PATH_INFO"] = target;
    // PATH_TRANSLATED = /cgi-bin/test.py
    envMap["PATH_TRANSLATED"] = server.getCgiDir() + target;
    envMap["SERVER_PORT"] = server.getServerPortString();
    envMap["SERVER_NAME"] = server.getServerName();
    envMap["SERVER_PROTOCOL"] = SERVER_PROTOCOL;
    envMap["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
    envMap["HTTP_USER_AGENT"] = request.getUserAgent();
    envMap["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
}

void CgiHandler::printEnv()
{
    for (auto env : envMap)
    {
        std::cout << env.first << ": " << env.second << std::endl;
    }
}

/* Create a new process to execute the CGI script:
- open cgi pipe
- fork and execute in the child process
- wait for the child process to finish
- close the pipe
*/
void CgiHandler::createCgiProcess()
{
}

void CgiHandler::executeCgiScript()
{
    std::vector<std::string> cgiEnvStr;
    for (auto env : envMap)
    {
        const std::string envStr = env.first + "=" + env.second;
        cgiEnvStr.push_back(envStr);
    }
    std::vector<const char *> cgiEnv = createCgiEnvCharStr(cgiEnvStr);
    char **cgiEnvp = const_cast<char **>(cgiEnv.data());
    while (*cgiEnvp)
    {
        std::cout << *cgiEnvp << std::endl;
        cgiEnvp++;
    }
    // fork
    // execve
    // waitpid
}

std::vector<const char *> CgiHandler::createCgiEnvCharStr(std::vector<std::string> &cgiEnvStr)
{
    std::vector<const char *> cgiEnv;
    for (const std::string &env : cgiEnvStr)
    {
        cgiEnv.push_back(env.c_str());
    }
    cgiEnv.push_back(nullptr);
    return cgiEnv;
}