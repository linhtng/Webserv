#include "CgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::CgiHandler(const Request &request, std::unordered_map<std::string, std::string> &cgiParams) : cgiExitStatus(HttpStatusCode::UNDEFINED_STATUS)
{
	config = request.getConfig();
	scriptName = cgiParams["fileName"];
	cgiOutput = "";
	messageBody = request.getBody();
	messageBodyStr.reserve(messageBody.size());
	for (const auto &byte : messageBody)
	{
		messageBodyStr.push_back(static_cast<char>(byte));
	}
}

CgiHandler::~CgiHandler()
{
	closeCgiPipes();
}

void CgiHandler::initializeCgi(const Request &request, std::unordered_map<std::string, std::string> &cgiParams)
{
	cgiBinDir = config.getCgiDir();
	if (cgiBinDir.empty())
	{
		cgiExitStatus = HttpStatusCode::NOT_FOUND;
		throw std::runtime_error("Error: CGI bin not found. Make sure the directory exists.\n");
	}
	setCgiExecutor(config);
	setupCgiEnv(request, config, cgiParams);
	if (FileSystemUtils::pathExistsAndAccessible(envMap["PATH_TRANSLATED"]) == false)
	{
		cgiExitStatus = HttpStatusCode::NOT_FOUND;
		throw std::runtime_error("Error: CGI script not accessible: " + envMap["PATH_TRANSLATED"] + "\n");
	}
}

void CgiHandler::setCgiExecutor(const ConfigData &server)
{
	std::unordered_map<std::string, std::string> cgiExtenExecutorMap = server.getCgiExtenExecutorMap();
	for (auto &extenExecutor : cgiExtenExecutorMap)
	{
		if (scriptName.find(extenExecutor.first) != std::string::npos)
		{
			cgiExecutorPathname = extenExecutor.second;
			break;
		}
	}
	if (cgiExecutorPathname.empty())
	{
		cgiExitStatus = HttpStatusCode::NOT_FOUND;
		throw std::runtime_error("Error: CGI executor not found to execute " + scriptName + "\n");
	}
}

void CgiHandler::setupCgiEnv(const Request &request, const ConfigData &server, std::unordered_map<std::string, std::string> &cgiParams)
{
	envMap["REQUEST_METHOD"] = request.getMethodStr();
	envMap["CONTENT_TYPE"] = request.getContentType();
	envMap["CONTENT_LENGTH"] = std::to_string(request.getContentLength());
	envMap["QUERY_STRING"] = cgiParams["queryParams"];
	std::cout << "scriptName: " << scriptName << std::endl;
	envMap["PATH_INFO"] = scriptName;
	std::cout << "cgiBinDir: " << cgiBinDir << std::endl;
	envMap["PATH_TRANSLATED"] = StringUtils::joinPath(cgiBinDir, scriptName);
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
		cgiExitStatus = HttpStatusCode::INTERNAL_SERVER_ERROR;
		Logger::log(ERROR, ERROR_MESSAGE, "Error: pipe() failed\n");
		return;
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		closeCgiPipes();
		cgiExitStatus = HttpStatusCode::INTERNAL_SERVER_ERROR;
		Logger::log(ERROR, ERROR_MESSAGE, "Error: fork() failed\n");
		return;
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
		Logger::log(ERROR, ERROR_MESSAGE, "Error: CGI script timed out");
		cgiExitStatus = HttpStatusCode::REQUEST_TIMEOUT;
		return;
	}
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status) == CGI_EXIT_SUCCESS)
			cgiExitStatus = HttpStatusCode::OK;
		else
			cgiExitStatus = HttpStatusCode::BAD_REQUEST;
	}
	else if (WIFSIGNALED(status))
	{
		cgiExitStatus = HttpStatusCode::BAD_REQUEST;
	}
}

void CgiHandler::readCgiOutput()
{
	char buffer[CGI_OUTPUT_BUFFER_SIZE];
	ssize_t bytesRead;
	std::stringstream ss;
	while ((bytesRead = read(dataFromCgiPipe[READ_END], buffer, sizeof(buffer))) > 0)
	{
		buffer[bytesRead] = '\0'; // null terminate the buffer
		ss << buffer;
	}
	if (bytesRead == -1)
	{
		Logger::log(ERROR, ERROR_MESSAGE, "Error: read() failed\n");
		cgiExitStatus = HttpStatusCode::INTERNAL_SERVER_ERROR;
		return;
	}
	cgiOutput = ss.str();
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
	std::vector<std::string> cgiEnvStr;
	for (auto env : envMap)
	{
		const std::string envStr = env.first + "=" + env.second;
		cgiEnvStr.push_back(envStr);
	}
	std::vector<const char *> cgiEnv = createCgiEnvCharStr(cgiEnvStr);
	char **cgiEnvp = const_cast<char **>(cgiEnv.data());

	std::vector<const char *> cgiArgVec;
	cgiArgVec.push_back(cgiExecutorPathname.c_str());
	cgiArgVec.push_back(envMap["PATH_INFO"].c_str());
	cgiArgVec.push_back(nullptr);
	char **cgiArgv = const_cast<char **>(cgiArgVec.data());

	chdir(cgiBinDir.c_str());
	execve(cgiArgv[0], cgiArgv, cgiEnvp);
	Logger::log(ERROR, ERROR_MESSAGE, "Error: execve() failed\n");
	cgiExitStatus = HttpStatusCode::INTERNAL_SERVER_ERROR;
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

HttpStatusCode CgiHandler::getCgiExitStatus()
{
	return cgiExitStatus;
}