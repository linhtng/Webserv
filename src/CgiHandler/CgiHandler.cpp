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

    // for (auto env : envMap)
    // {
    //     const std::string envStr = env.first + "=" + env.second;
    //     // std::cout << "New env:" << std::endl;
    //     // std::cout << envStr << std::endl;
    //     // std::cout << envStr.c_str() << std::endl;
    //     cgiEnv.push_back(envStr.c_str());
    // }
    // cgiEnv.push_back(nullptr);
    // for (const char *env : cgiEnv)
    // {
    //     if (env != nullptr)
    //         std::cout << env << std::endl;
    // }
    std::vector<std::string> cgiEnvStr;
    for (auto env : envMap)
    {
        const std::string envStr = env.first + "=" + env.second;
        cgiEnvStr.push_back(envStr);
    }
}

std::vector<const char *> CgiHandler::createCgiEnvCharStr()
{
    std::vector<const char *> cgiEnvChar;
    for (const std::string &env : cgiEnv)
    {
        cgiEnvChar.push_back(env.c_str());
    }
    cgiEnvChar.push_back(nullptr);
    return cgiEnvChar;
}

void CgiHandler::printEnv()
{
    for (const char *env : cgiEnv)
    {
        if (env == nullptr)
        {
            break;
        }
        std::cout << env << std::endl;
    }
}