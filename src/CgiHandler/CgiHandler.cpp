#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const Request &request, Location &cgiBin)
{
    parseCgiRequest(request, cgiBin);
}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::parseCgiRequest(const Request &request, Location &cgiBin)
{
    env["AUTH_TYPE"] = "Basic";
    env["REQUEST_METHOD"] = request.getMethod();
    env["CONTENT_TYPE"] = request.getContentType();
    env["CONTENT_LENGTH"] = request.getContentLength();
    // env["QUERY_STRING"] = request.getQuery();
    env["SCRIPT_NAME"] = request.getTarget();
    env["PATH_INFO"] = setPathInfo(request, cgiBin);
    // env["PATH_TRANSLATED"] = request.getConfig().ge()  +request.getTarget();
    env["SERVER_PORT"] = request.getConfig().getServerPort();
    env["SERVER_NAME"] = request.getConfig().getServerName();
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["GATEWAY_INTERFACE"] = GATEWAY_INTERFACE;
    env["REDIRECT_STATUS"] = "200";
    env["HTTP_HOST"] = request.getHost();
    env["HTTP_USER_AGENT"] = request.getUserAgent();
}

// PATH_INFO = full path to the requested file, i.e. root of cgi-bin + target
std::string CgiHandler::setPathInfo(const Request &request, Location &cgiBin)
{
    std::string path_info = cgiBin.getLocationRoot() + cgiBin.getLocationRoute() + request.getTarget();
    std::cout << "PATH_INFO: " << path_info << std::endl;
    return path_info;
}
