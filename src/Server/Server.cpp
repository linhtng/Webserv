#include "Server.hpp"

Server::Server(ConfigData &config) : serverFd(-1)
{
	configs.push_back(config);
	host = config.getServerHost();
	port = config.getServerPort();
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = inet_addr(host.c_str());
	std::cout << YELLOW << "constructor of server is called" << RESET << std::endl;
}

Server::~Server()
{
	std::cout << YELLOW << "destructor of server is called" << RESET << std::endl;
}

void Server::setUpServerSocket()
{
	int opt;

	opt = 1;
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create socket file descriptor
		throw SocketCreationException();
	try
	{
		if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, // set file descriptor to be reuseable
					   sizeof(opt)) < 0)
			throw SocketSetOptionException();
		if (fcntl(serverFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
			throw SocketSetNonBlockingException();
		if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) // bind the socket to the address and port number
			throw SocketBindingException();
		if (listen(serverFd, BACKLOG) < 0) // set server socket in passive mode
			throw SocketListenException();
	}
	catch (std::exception &e)
	{
		close(serverFd);
		throw;
	}
}

int Server::acceptNewConnection()
{
	int clientFd;
	struct sockaddr_in clientAddress;
	socklen_t clientAddrlen = sizeof(clientAddress);

	clientFd = accept(serverFd, (struct sockaddr *)&clientAddress, &clientAddrlen);
	if (clientFd < 0)
	{
		Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to accept client socket", host.c_str(), port);
		return (clientFd);
	}
	clients[clientFd] = std::make_unique<Client>(clientAddress);
	Logger::log(e_log_level::INFO, CLIENT, "New connection from Client %s:%d to Server %s:%d",
				inet_ntoa(getClientIPv4Address(clientFd)),
				ntohs(getClientPortNumber(clientFd)),
				host.c_str(),
				port);
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
	{
		createAndSendErrorResponse(HttpStatusCode::INTERNAL_SERVER_ERROR, clientFd);
		close(clientFd);
		throw SocketSetNonBlockingException();
	}
	return (clientFd);
}

Server::RequestStatus Server::receiveRequest(int const &clientFd)
{
	RequestStatus requestStatus = clients[clientFd]->isNewRequest()
									  ? receiveRequestHeader(clientFd)
									  : receiveRequestBody(clientFd);

	if (requestStatus == REQUEST_CLIENT_DISCONNECT || requestStatus == BODY_IN_CHUNK)
		return (requestStatus);
	else if (requestStatus == SERVER_ERROR || requestStatus == BAD_REQUEST || requestStatus == PAYLOAD_TOO_LARGE)
	{
		if (requestStatus == SERVER_ERROR)
			clients[clientFd]->createErrorRequest(configs, HttpStatusCode::INTERNAL_SERVER_ERROR);
		else if (requestStatus == BAD_REQUEST)
			clients[clientFd]->createErrorRequest(configs, HttpStatusCode::BAD_REQUEST);
		else if (requestStatus == PAYLOAD_TOO_LARGE)
			clients[clientFd]->createErrorRequest(configs, HttpStatusCode::PAYLOAD_TOO_LARGE);
		clients[clientFd]->setIsConnectionClose(true);
	}
	clients[clientFd]->createResponse();
	return (READY_TO_WRITE);
}

// if the request is not created yet, create the request with the request header
Server::RequestStatus Server::receiveRequestHeader(int const &clientFd)
{
	std::string requestHeader;
	std::vector<std::byte> requestBodyBuf;

	RequestStatus requestStatus = formRequestHeader(clientFd, requestHeader, requestBodyBuf); // return HEADER_DELIMITER_FOUND or PAYLOAD_TOO_LARGE or BAD_REQUEST or REQUEST_CLIENT_DISCONNECT or SERVER_ERROR
	if (requestStatus != HEADER_DELIMITER_FOUND)
		return (requestStatus);

	// std::cout << YELLOW << "--------------requestHeader---------------" << std::endl
	// 		  << requestHeader << RESET << std::endl;

	clients[clientFd]->createRequest(requestHeader, configs); // create request object
	clients[clientFd]->appendToBodyBuf(requestBodyBuf);
	Request request = clients[clientFd]->getRequest();
	Logger::log(e_log_level::INFO, CLIENT, "Request from Client %s:%d - Method: %d, Target: %s",
				inet_ntoa(getClientIPv4Address(clientFd)),
				ntohs(getClientPortNumber(clientFd)),
				request.getMethod(),
				request.getTarget().c_str());

	if (request.isBodyExpected())
		return (processRequestHeaderBuf(clientFd));
	return (READY_TO_WRITE);
}

Server::RequestStatus Server::formRequestHeader(int const &clientFd, std::string &requestHeader, std::vector<std::byte> &requestBodyBuf)
{
	ssize_t bytes;
	char buf[MAX_REQUEST_HEADER_LENGTH];

	if ((bytes = recv(clientFd, buf, sizeof(buf), 0)) > 0)
	{
		requestHeader.append(buf, bytes);
		// std::cout << YELLOW << "--------------requestHeader loop---------------" << std::endl
		// 					<< requestHeader << RESET << std::endl;
		size_t delimiterPos = requestHeader.find(CRLF CRLF);
		if (delimiterPos != std::string::npos)
		{
			for (size_t i = delimiterPos + sizeof(CRLF CRLF) - 1; i < requestHeader.size(); ++i)
				requestBodyBuf.push_back(static_cast<std::byte>(requestHeader[i]));
			requestHeader.resize(delimiterPos);
			return (HEADER_DELIMITER_FOUND);
		}
		else
		{
			if (requestHeader.size() == MAX_REQUEST_HEADER_LENGTH) // the header is larger than the max header length
				return (PAYLOAD_TOO_LARGE);
			else // cannot find delimiter
				return (BAD_REQUEST);
		}
	}
	else
	{
		if (bytes == 0)
		{
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)));
			return (REQUEST_CLIENT_DISCONNECT);
		}
		else
		{
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to receive request from Client %s:%d", host.c_str(), port,
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)));
			return (SERVER_ERROR);
		}
	}
}

// process any remaining data in the body buffer from request header
Server::RequestStatus Server::processRequestHeaderBuf(int const &clientFd)
{
	Request request = clients[clientFd]->getRequest();

	if (clients[clientFd]->getBodyBuf().size() == 0)
		return (BODY_IN_CHUNK);
	else
	{
		if (request.isChunked())
			return (processChunkData(clientFd));
		else
		{
			clients[clientFd]->appendToRequestBody(clients[clientFd]->getBodyBuf());
			clients[clientFd]->clearBodyBuf();
			size_t bodySize = clients[clientFd]->getRequestBody().size();
			size_t contentLength = request.getContentLength();
			if (bodySize < contentLength)
				return (BODY_IN_CHUNK);
			else if (bodySize == contentLength)
				return (READY_TO_WRITE);
			else
				return (BAD_REQUEST);
		}
	}
}

Server::RequestStatus Server::receiveRequestBody(int const &clientFd)
{
	Request request = clients[clientFd]->getRequest();
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	if ((bytes = recv(clientFd, buf, sizeof(buf), 0)) > 0)
	{
		if (request.getStatusCode() == HttpStatusCode::UNDEFINED_STATUS)
			return (request.isChunked()
						? formRequestBodyWithChunk(clientFd, buf, bytes)
						: formRequestBodyWithContentLength(clientFd, buf, bytes));
		else
		{
			clients[clientFd]->setIsConnectionClose(true);
			return (READY_TO_WRITE);
		}
	}
	else
	{
		if (bytes == 0)
		{
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)));
			return (REQUEST_CLIENT_DISCONNECT);
		}
		else
		{
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to receive request from Client %s:%d",
						host.c_str(),
						port,
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)));
			return (SERVER_ERROR);
		}
	}
}

Server::RequestStatus Server::formRequestBodyWithContentLength(int const &clientFd, char readBuf[], ssize_t const &bytes)
{
	clients[clientFd]->appendToRequestBody(readBuf, bytes);
	size_t bodySize = clients[clientFd]->getRequestBody().size();
	size_t contentLength = clients[clientFd]->getRequest().getContentLength();
	if (bodySize < contentLength)
		return (BODY_IN_CHUNK);
	else if (bodySize == contentLength)
		return (READY_TO_WRITE);
	else
		return (BAD_REQUEST);
}

Server::RequestStatus Server::formRequestBodyWithChunk(int const &clientFd, char readBuf[], ssize_t const &bytes)
{
	if (!clients[clientFd]->getBodyBuf().empty()) // process any remaining data from the former chunk
	{
		RequestStatus requestStatus = processChunkData(clientFd); // return READY_TO_WRITE or BAD_REQUEST or BODY_IN_CHUNK
		if (requestStatus != BODY_IN_CHUNK)
			return (requestStatus);
	}

	clients[clientFd]->appendToBodyBuf(readBuf, bytes);
	return (processChunkData(clientFd));
}

Server::RequestStatus Server::processChunkData(int const &clientFd)
{
	if (clients[clientFd]->getChunkSize() == 0) // if not yet parse the chunk size or the chunk size is 0
	{
		RequestStatus requestStatus = extractChunkSize(clientFd); // return READY_TO_WRITE or BAD_REQUEST or BODY_IN_CHUNK or PARSED_CHUNK_SIZE
		if (requestStatus != PARSED_CHUNK_SIZE)
			return (requestStatus);
	}

	clients[clientFd]->appendToRequestBody(clients[clientFd]->getBodyBuf());
	clients[clientFd]->clearBodyBuf();

	std::vector<std::byte> body = clients[clientFd]->getRequestBody();

	if (body.size() >= clients[clientFd]->getBytesToReceive() + (sizeof(CRLF) - 1)) // last buffer for the chunk
	{
		std::string final_chunk = "0" CRLF CRLF;
		if (body.size() >= final_chunk.length() && std::memcmp(body.data() + body.size() - final_chunk.length(), final_chunk.data(), final_chunk.length()) == 0) // if the chunk contains final chunk at the end
		{
			if (body.size() == final_chunk.size() && std::memcmp(body.data(), final_chunk.data(), final_chunk.length()) == 0)
				return (READY_TO_WRITE);
			RequestStatus requestStatus;
			do
			{
				clients[clientFd]->appendToRequestBody(clients[clientFd]->getBodyBuf());
				clients[clientFd]->clearBodyBuf();
				body = clients[clientFd]->getRequestBody();
				if (static_cast<char>(body[clients[clientFd]->getBytesToReceive()]) != '\r' || static_cast<char>(body[clients[clientFd]->getBytesToReceive() + 1]) != '\n')
					return (BAD_REQUEST);
				std::vector<std::byte> bodyBuf(body.begin() + clients[clientFd]->getBytesToReceive() + (sizeof(CRLF) - 1), body.end());
				clients[clientFd]->appendToBodyBuf(bodyBuf);
				clients[clientFd]->resizeRequestBody(clients[clientFd]->getBytesToReceive());
				clients[clientFd]->setChunkSize(0);
				requestStatus = extractChunkSize(clientFd);
			} while (requestStatus == PARSED_CHUNK_SIZE);
			return (requestStatus);
		}
		else
		{
			if (static_cast<char>(body[clients[clientFd]->getBytesToReceive()]) != '\r' || static_cast<char>(body[clients[clientFd]->getBytesToReceive() + 1]) != '\n')
				return (BAD_REQUEST);
			std::vector<std::byte> bodyBuf(body.begin() + clients[clientFd]->getBytesToReceive() + (sizeof(CRLF) - 1), body.end());
			clients[clientFd]->appendToBodyBuf(bodyBuf);
			clients[clientFd]->resizeRequestBody(clients[clientFd]->getBytesToReceive());
			clients[clientFd]->setChunkSize(0);
			return (BODY_IN_CHUNK);
		}
	}
	return (BODY_IN_CHUNK);
}

Server::RequestStatus Server::extractChunkSize(int const &clientFd)
{
	std::vector<std::byte> bodyBuf = clients[clientFd]->getBodyBuf();

	std::string final_chunk = "0" CRLF CRLF;
	if (std::memcmp(bodyBuf.data(), final_chunk.data(), std::min(final_chunk.length(), bodyBuf.size())) == 0)
	{
		if (bodyBuf.size() == final_chunk.length())
			return (READY_TO_WRITE);
		else if (bodyBuf.size() > final_chunk.length())
			return (BAD_REQUEST);
		else
			return (BODY_IN_CHUNK);
	}

	std::vector<std::byte> delimiter = {std::byte('\r'), std::byte('\n')};
	auto delimiterPos = std::search(bodyBuf.begin(), bodyBuf.end(), delimiter.begin(), delimiter.end());
	if (delimiterPos != bodyBuf.end())
	{
		std::string chunk_size;
		for (auto it = bodyBuf.begin(); it != delimiterPos; ++it)
			chunk_size.push_back(static_cast<char>(*it));
		if (chunk_size.length() > 0 && std::all_of(chunk_size.begin(), chunk_size.end(), ::isxdigit))
		{
			clients[clientFd]->setChunkSize(std::stoi(chunk_size, nullptr, 16));
			clients[clientFd]->setBytesToReceive(clients[clientFd]->getBytesToReceive() + clients[clientFd]->getChunkSize());
			clients[clientFd]->eraseBodyBuf(0, std::distance(bodyBuf.begin(), delimiterPos) + (sizeof(CRLF) - 1)); // remove the chunk size from body buffer
			return (PARSED_CHUNK_SIZE);
		}
		else
			return (BAD_REQUEST);
	}
	else
	{
		std::string bodyBufStr;
		for (auto &ch : bodyBuf)
			bodyBufStr.push_back(static_cast<char>(ch));
		if (!std::all_of(bodyBufStr.begin(), bodyBufStr.end(), ::isxdigit))
			return (BAD_REQUEST);
		else
			return (BODY_IN_CHUNK);
	}
}

Server::ResponseStatus Server::sendResponse(int const &clientFd)
{
	//--------------------------------------------------------------
	// TODO - to save the file in Request/Response Class
	// std::ofstream ofs("test.txt");
	// std::vector<std::byte> body = clients[clientFd]->getRequestBody();
	// const std::byte *dataPtr = body.data();
	// std::size_t dataSize = body.size();
	// ofs.write(reinterpret_cast<const char *>(dataPtr), dataSize);
	// ofs.close();
	//--------------------------------------------------------------

	Response response = clients[clientFd]->getResponse();
	std::vector<std::byte> formatedResponse = response.formatResponse();

	ssize_t bytes;
	if ((bytes = send(clientFd, &(*(formatedResponse.begin() + clients[clientFd]->getBytesSent())), std::min(formatedResponse.size() - clients[clientFd]->getBytesSent(), static_cast<size_t>(BUFFER_SIZE)), 0)) > 0)
	{
		clients[clientFd]->setBytesSent(clients[clientFd]->getBytesSent() + bytes);
		if (clients[clientFd]->getBytesSent() < formatedResponse.size())
			return (RESPONSE_IN_CHUNK);
		else
		{
			Logger::log(e_log_level::INFO, CLIENT, "Response sent to Client %s:%d - Status: %d",
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)),
						response.getStatusCode());
			if (clients[clientFd]->getRequest().getConnection() == ConnectionValue::CLOSE || clients[clientFd]->getIsConnectionClose() == true)
				return (RESPONSE_DISCONNECT_CLIENT);
			clients[clientFd]->removeRequest();
			clients[clientFd]->removeResponse();
			return (KEEP_ALIVE); // keep the connection alive by default
		}
	}
	else
	{
		if (bytes == 0)
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)));
		else
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to send response to Client %s:%d",
						host.c_str(),
						port,
						inet_ntoa(getClientIPv4Address(clientFd)),
						ntohs(getClientPortNumber(clientFd)));
		return (RESPONSE_DISCONNECT_CLIENT);
	}
}

void Server::createAndSendErrorResponse(HttpStatusCode const &statusCode, int const &clientFd)
{
	clients[clientFd]->createErrorRequest(configs, statusCode);
	clients[clientFd]->createResponse();
	sendResponse(clientFd);
}

int const &Server::getServerFd() const
{
	return (serverFd);
}

std::string const &Server::getHost()
{
	return (host);
}

int const &Server::getPort()
{
	return (port);
}

unsigned short int const &Server::getClientPortNumber(int const &clientFd)
{
	return (clients[clientFd]->getPortNumber());
}

in_addr const &Server::getClientIPv4Address(int const &clientFd)
{
	return (clients[clientFd]->getIPv4Address());
}

void Server::appendConfig(ConfigData const &config)
{
	configs.push_back(config);
}

void Server::removeClient(int const &clientFd)
{
	Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d is removed",
				inet_ntoa(getClientIPv4Address(clientFd)),
				ntohs(getClientPortNumber(clientFd)));
	clients.erase(clientFd);
}

const char *Server::SocketCreationException::what() const throw()
{
	return "Server::Fail to create socket";
}

const char *Server::SocketBindingException::what() const throw()
{
	return "Server::Fail to bind socket";
}

const char *Server::SocketListenException::what() const throw()
{
	return "Server::Fail to set socket in passive mode";
}

const char *Server::SocketSetNonBlockingException::what() const throw()
{
	return "Server::Fail to set socket as non-blocking";
}

const char *Server::SocketSetOptionException::what() const throw()
{
	return "Server::Fail to set socket options";
}
