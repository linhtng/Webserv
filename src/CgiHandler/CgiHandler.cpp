#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const Request &request, const ConfigData &server)
{
    cgiExecutorPathname = server.getCgiExecutor();
    cgiBinDir = server.getCgiDir();
    if (cgiExecutorPathname.empty() || cgiBinDir.empty())
    {
        throw std::runtime_error("Error: CGI executor and/or bin not found\n");
    }
    setupCgiEnv(request, server);
    cgiOutput = "";
    messageBody = request.getBody();
    messageBodyStr.reserve(messageBody.size());
    for (const auto &byte : messageBody)
    {
        messageBodyStr.push_back(static_cast<char>(byte));
    }
    // std::cout << "messageBodyStr: " << messageBodyStr << std::endl;
    // printEnv();
}

CgiHandler::~CgiHandler()
{
    closeCgiPipes();
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
    envMap["PATH_TRANSLATED"] = cgiBinDir + target;
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
    if (pipe(dataToCgiPipe) == -1 || pipe(dataFromCgiPipe) == -1)
    {
        closeCgiPipes();
        throw std::runtime_error("Error: pipe() failed");
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        closeCgiPipes();
        throw std::runtime_error("Error: fork() failed");
    }
    if (pid == 0) // child process
    {
        // redirect stdin and stdout
        dup2(dataToCgiPipe[READ_END], STDIN_FILENO);
        dup2(dataFromCgiPipe[WRITE_END], STDOUT_FILENO);

        // write message body to cgi process's stdin
        write(dataToCgiPipe[WRITE_END], messageBodyStr.c_str(), messageBodyStr.size());

        closeCgiPipes();

        executeCgiScript();
    }
    // parent process
    else if (pid > 0)
    {
        close(dataToCgiPipe[WRITE_END]);
        close(dataFromCgiPipe[WRITE_END]);
        readCgiOutput();
        closeCgiPipes();
    }
}

void CgiHandler::readCgiOutput()
{
    char buffer[CGI_OUTPUT_BUFFER_SIZE];
    ssize_t bytesRead;
    std::stringstream ss;
    while ((bytesRead = read(dataFromCgiPipe[READ_END], buffer, sizeof(buffer))) > 0)
    {
        ss.write(buffer, bytesRead);
    }
    cgiOutput = ss.str();
    std::cout << "cgiOutput: " << cgiOutput << std::endl;
}

void CgiHandler::closePipeEnd(int pipeFd)
{
    if (pipeFd == -1)
        return;
    close(pipeFd);
}

void CgiHandler::closeCgiPipes()
{
    closePipeEnd(dataToCgiPipe[READ_END]);
    closePipeEnd(dataToCgiPipe[WRITE_END]);
    closePipeEnd(dataFromCgiPipe[READ_END]);
    closePipeEnd(dataFromCgiPipe[WRITE_END]);
}

void printCharArr(char **arr)
{
    while (*arr != nullptr)
    {
        std::cout << *arr << std::endl;
        arr++;
    }
}

void CgiHandler::executeCgiScript()
{
    /* setup char *const envp[] for execve */
    std::vector<std::string> cgiEnvStr;
    for (auto env : envMap)
    {
        const std::string envStr = env.first + "=" + env.second;
        cgiEnvStr.push_back(envStr);
    }
    std::vector<const char *> cgiEnv = createCgiEnvCharStr(cgiEnvStr);
    char **cgiEnvp = const_cast<char **>(cgiEnv.data());
    // printCharArr(cgiEnvp); // Debug

    /* setup char *const argv[] for execve */
    std::vector<const char *> cgiArgVec;
    cgiArgVec.push_back(cgiExecutorPathname.c_str());
    cgiArgVec.push_back(envMap["PATH_INFO"].c_str());
    cgiArgVec.push_back(nullptr);
    char **cgiArgv = const_cast<char **>(cgiArgVec.data());
    // printCharArr(cgiArgv); // Debug

    chdir(cgiBinDir.c_str());
    execve(cgiArgv[0], cgiArgv, cgiEnvp);
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

/* Getters */

std::string CgiHandler::getCgiOutput()
{
    return cgiOutput;
}