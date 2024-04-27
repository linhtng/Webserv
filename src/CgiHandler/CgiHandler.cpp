#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const std::string &path, const Request &request)
{
    cgiPath = path;
    parseCgiRequest(request);
}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::parseCgiRequest(const Request &request)
{
    env["REQUEST_METHOD"] = request.getMethod();
    env["CONTENT_TYPE"] = request.getContentType();
    env["CONTENT_LENGTH"] = request.getContentLength();
    // env["QUERY_STRING"] = request.getQuery();
    env["SCRIPT_NAME"] = request.getTarget();
    env["PATH_INFO"] = CGI_PATH + request.getTarget();
}