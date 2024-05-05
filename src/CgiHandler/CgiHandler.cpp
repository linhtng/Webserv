#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const Request &request, const ConfigData &server)
{
    setCgiExecutor(request, server);
    cgiBinDir = server.getCgiDir();
    if (cgiExecutorPathname.empty() || cgiBinDir.empty())
    {
        throw std::runtime_error("Error: CGI executor and/or bin not found\n");
    }
    setupCgiEnv(request, server);
    if (FileSystemUtils::pathExistsAndAccessible(envMap["PATH_TRANSLATED"]) == false)
    {
        throw std::runtime_error("Error: CGI script not accessible: " + envMap["PATH_TRANSLATED"] + "\n");
    }
    cgiOutput = "";
    messageBody = request.getBody();
    messageBodyStr.reserve(messageBody.size());
    for (const auto &byte : messageBody)
    {
        messageBodyStr.push_back(static_cast<char>(byte));
    }
    // std::cout << "messageBodyStr: " << messageBodyStr << std::endl;
    printEnv();
}

CgiHandler::~CgiHandler()
{
    closeCgiPipes();
}

void CgiHandler::setCgiExecutor(const Request &request, const ConfigData &server)
{
    scriptName = StringUtils::extractPathPreQuery(request.getTarget());
    std::unordered_map<std::string, std::string> cgiExtenExecutorMap = server.getCgiExtenExecutorMap();
    for (auto &extenExecutor : cgiExtenExecutorMap)
    {
        std::cout << "extenExecutor.first: " << extenExecutor.first << std::endl;
        std::cout << "extenExecutor.second: " << extenExecutor.second << std::endl;
        if (scriptName.find(extenExecutor.first) != std::string::npos)
        {
            cgiExecutorPathname = extenExecutor.second;
            break;
        }
    }
    std::cout << "cgiExecutorPathname: " << cgiExecutorPathname << std::endl;
}

void CgiHandler::setupCgiEnv(const Request &request, const ConfigData &server)
{
    envMap["REQUEST_METHOD"] = request.getMethodStr();
    envMap["CONTENT_TYPE"] = request.getContentType();
    envMap["CONTENT_LENGTH"] = std::to_string(request.getContentLength());
    envMap["QUERY_STRING"] = StringUtils::queryStr(request.getTarget());
    // PATH_INFO = test.py
    envMap["PATH_INFO"] = scriptName;
    // PATH_TRANSLATED = /cgi-bin/test.py
    envMap["PATH_TRANSLATED"] = cgiBinDir + scriptName;
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
        cgiTimeout(pid);
        readCgiOutput();
        closeCgiPipes();
    }
}

void CgiHandler::cgiTimeout(pid_t pid)
{
    int status;
    int waitResult;

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    while (true)
    {
        waitResult = waitpid(pid, &status, WNOHANG);
        if (waitResult != 0) // child process has exited
            break;
        auto now = std::chrono::high_resolution_clock::now();
        elapsed = now - start;
        if (elapsed.count() >= CGI_TIMEOUT) // timeout has been reached
            break;
    }
    if (waitResult == 0) // child process has not exited after timeout
    {
        // Kill the child process
        kill(pid, SIGKILL);
        std::cerr << "Timeout: The child process has been killed." << std::endl;
    }
    if (WIFEXITED(status))
    {
        cgiExitStatus = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        cgiExitStatus = WTERMSIG(status);
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

int CgiHandler::getCgiExitStatus()
{
    return cgiExitStatus;
}