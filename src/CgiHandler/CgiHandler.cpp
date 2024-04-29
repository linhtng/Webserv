#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const Request &request, const ConfigData &server)
{
    parseCgiRequest(request, server);
}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::parseCgiRequest(const Request &request, const ConfigData &server)
{
    std::string target = request.getTarget();
    env["REQUEST_METHOD"] = request.getMethod();
    env["CONTENT_TYPE"] = request.getContentType();
    env["CONTENT_LENGTH"] = request.getContentLength();
    // env["QUERY_STRING"] = request.getQuery();
    // PATH_INFO = test.py
    env["PATH_INFO"] = target;
    // PATH_TRANSLATED = /cgi-bin/test.py
    env["PATH_TRANSLATED"] = server.getCgiDir() + target;
    env["SERVER_PORT"] = server.getServerPortString();
    env["SERVER_NAME"] = server.getServerName();
    env["SERVER_PROTOCOL"] = SERVER_PROTOCOL;
    env["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
    env["REMOTE_ADDR"] = request.getHost();
    env["HTTP_USER_AGENT"] = request.getUserAgent();
    env["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
}

void CgiHandler::printEnv()
{
    for (auto &pair : env)
    {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
}