#include "Server.hpp"
#include "../config_parser/ConfigParser.hpp"
#include <cstring>
#include <fstream>

Server::Server(ConfigData &config)
	: server_fd(-1)
{
	configs.push_back(config);
	host = config.getServerHost();
	port = config.getServerPort();
	std::cout << YELLOW << "constructor of server is called" << RESET << std::endl;
}

Server::~Server()
{
	std::cout << YELLOW << "destructor of server is called" << RESET << std::endl;
}

// set up server socket
void Server::setUpServerSocket()
{
	int opt;

	opt = 1;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create socket file descriptor
		throw SocketCreationException();
	std::cout << "server_fd: " << server_fd << std::endl;
	try
	{
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, // set file descriptor to be reuseable
					   sizeof(opt)) < 0)
			throw SocketSetOptionException();
		if (fcntl(server_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
			throw SocketSetNonBlockingException();
		address.sin_family = AF_INET;
		std::cout << "server port: " << port << std::endl;
		address.sin_port = htons(port);
		std::cout << "server host: " << host << std::endl;
		address.sin_addr.s_addr = inet_addr(host.c_str());
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) // bind the socket to the address and port number
			throw SocketBindingException();
		if (listen(server_fd, BACKLOG) < 0) // set server socket in passive mode
			throw SocketListenException();
	}
	catch (std::exception &e)
	{
		close(server_fd);
		throw;
	}
}

int Server::acceptNewConnection()
{
	int client_fd;

	struct sockaddr_in client_address;
	socklen_t client_addrlen = sizeof(client_address);
	client_fd = accept(server_fd,
								(struct sockaddr *)&client_address,
								&client_addrlen);
	if (client_fd < 0)
	{
		Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to accept socket", host.c_str(), port);
		return (client_fd);
	}
	clients[client_fd] = std::make_unique<Client>(client_address, client_addrlen);
	Logger::log(e_log_level::INFO, CLIENT, "New connection from Client %s:%d to Server %s:%d",
							inet_ntoa(getClientIPv4Address(client_fd)),
							ntohs(getClientPortNumber(client_fd)),
							host.c_str(),
							port);
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
		throw SocketSetNonBlockingException();
	return (client_fd);
}

Server::RequestStatus Server::receiveRequest(int const &client_fd)
{
	RequestStatus request_status = BODY_EXPECTED;

	if (clients[client_fd]->isNewRequest()) // if the request is not created yet, create the request with the request header
	{
		std::string request_header;
		std::vector<std::byte> request_body_buf;

		request_status = formRequestHeader(client_fd, request_header, request_body_buf);
		if (request_status == REQUEST_DISCONNECT_CLIENT)
			return request_status;
		if (request_status == BAD_HEADER || request_status == SERVER_ERROR)
		{
			clients[client_fd]->setIsConnectionClose(true);
			if (request_status == SERVER_ERROR)
				clients[client_fd]->createErrorRequest(configs, HttpStatusCode::INTERNAL_SERVER_ERROR);
			else
			{
				if (request_header.size() == MAX_HEADER_LENGTH)
					clients[client_fd]->createErrorRequest(configs, HttpStatusCode::PAYLOAD_TOO_LARGE);
				else
					clients[client_fd]->createErrorRequest(configs, HttpStatusCode::BAD_REQUEST);
			}
			clients[client_fd]->createResponse(); // create response object
			return (READY_TO_WRITE);
		}

		// std::cout << YELLOW << "--------------request_header---------------" << std::endl << request_header << RESET << std::endl;

		clients[client_fd]->createRequest(request_header, configs); // create request object
		Logger::log(e_log_level::INFO, CLIENT, "Request from Client %s:%d - Method: %d, Target: %s",
					inet_ntoa(getClientIPv4Address(client_fd)),
					ntohs(getClientPortNumber(client_fd)),
					clients[client_fd]->getRequestMethod(),
					clients[client_fd]->getRequestTarget().c_str());
		clients[client_fd]->appendToBodyBuf(request_body_buf);

		if (clients[client_fd]->isRequestBodyExpected() && request_body_buf.size() > 0)
		{
			if (clients[client_fd]->isRequestChunked())
			{
				RequestStatus request_status = processChunkData(client_fd);
				if (request_status == READY_TO_WRITE)
					clients[client_fd]->createResponse();
				return (request_status);
			}
			else
			{
				clients[client_fd]->appendToRequestBody(clients[client_fd]->getBodyBuf());
				clients[client_fd]->clearBodyBuf();
				size_t body_size = clients[client_fd]->getRequestBody().size();
				size_t content_length = clients[client_fd]->getRequestContentLength();
				if (body_size < content_length) // request body send in chunk
					return (BODY_IN_CHUNK);
				else if (body_size == content_length) // read till the end
				{
					clients[client_fd]->createResponse();
					return (READY_TO_WRITE);
				}
				else
					return (BAD_REQUEST);
			}
		}
		else if (clients[client_fd]->isRequestBodyExpected())
			return (BODY_IN_CHUNK);
		else
		{
			clients[client_fd]->createResponse(); // create response object
			return (READY_TO_WRITE);
		}
	}

	if (clients[client_fd]->isRequestBodyExpected() && clients[client_fd]->getRequestStatusCode() == HttpStatusCode::UNDEFINED_STATUS)
	{
		request_status = clients[client_fd]->isRequestChunked()
							 ? formRequestBodyWithChunk(client_fd)
							 : formRequestBodyWithContentLength(client_fd);
		if (request_status == REQUEST_DISCONNECT_CLIENT || request_status == BODY_IN_CHUNK)
			return (request_status);
	}

	if (request_status == BAD_REQUEST)
	{
		clients[client_fd]->setIsConnectionClose(true);
		clients[client_fd]->createErrorRequest(configs, HttpStatusCode::BAD_REQUEST);
	}
	else if (request_status == SERVER_ERROR)
	{
		clients[client_fd]->setIsConnectionClose(true);
		clients[client_fd]->createErrorRequest(configs, HttpStatusCode::INTERNAL_SERVER_ERROR);
	}
	clients[client_fd]->createResponse(); // create response object
	return (READY_TO_WRITE);
}

Server::RequestStatus Server::formRequestHeader(int const &client_fd, std::string &request_header, std::vector<std::byte> &request_body_buf)
{
	ssize_t bytes;
	char buf[MAX_HEADER_LENGTH];

	if ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		request_header.append(buf, bytes);
		// std::cout << YELLOW << "--------------request_header loop---------------" << std::endl << request_header << RESET << std::endl;
		size_t delimiter_pos = request_header.find(CRLF CRLF);
		if (delimiter_pos != std::string::npos)
		{
			size_t body_length = request_header.size() - delimiter_pos - (sizeof(CRLF CRLF) - 1);
			for (size_t i = 0; i < body_length; ++i)
				request_body_buf.push_back(static_cast<std::byte>(request_header[delimiter_pos + (sizeof(CRLF CRLF) - 1) + i]));
			request_header.resize(delimiter_pos);
			return (HEADER_DELIMITER_FOUND);
		}
		else
			return (BAD_HEADER); // cannot find delimiter or the header is larger than the max header length
	}
	else
	{
		if (bytes == 0)
		{
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
			return (REQUEST_DISCONNECT_CLIENT);
		}
		else
		{
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to receive request from Client %s:%d",
						host.c_str(),
						port,
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
			return (SERVER_ERROR);
		}
	}
}

Server::RequestStatus Server::formRequestBodyWithContentLength(int const &client_fd)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	if (!clients[client_fd]->getBodyBuf().empty()) // process any remaining data in the request body buffer
	{
		clients[client_fd]->appendToRequestBody(clients[client_fd]->getBodyBuf());
		clients[client_fd]->clearBodyBuf();
		size_t body_size = clients[client_fd]->getRequestBody().size();
		size_t content_length = clients[client_fd]->getRequestContentLength();
		if (body_size < content_length) // request body send in chunk
			return (BODY_IN_CHUNK);
		else if (body_size == content_length) // read till the end
			return (READY_TO_WRITE);
		else
			return (BAD_REQUEST);
	}

	if ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		clients[client_fd]->appendToRequestBody(buf, bytes);
		size_t body_size = clients[client_fd]->getRequestBody().size();
		size_t content_length = clients[client_fd]->getRequestContentLength();
		if (body_size < content_length) // request body send in chunk
			return (BODY_IN_CHUNK);
		else if (body_size == content_length) // read till the end
			return (READY_TO_WRITE);
		else
			return (BAD_REQUEST);
	}
	else
	{
		if (bytes == 0)
		{
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
			return (REQUEST_DISCONNECT_CLIENT);
		}
		else
		{
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to receive request from Client %s:%d",
						host.c_str(),
						port,
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
			return (SERVER_ERROR);
		}
	}
}

Server::RequestStatus Server::formRequestBodyWithChunk(int const &client_fd)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	if (!clients[client_fd]->getBodyBuf().empty()) // process any remaining data in the body buffer from request or last chunk
	{
		RequestStatus request_status = processChunkData(client_fd);
		if (request_status != BODY_IN_CHUNK)
			return (request_status);
	}

	if ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		clients[client_fd]->appendToBodyBuf(buf, bytes);
		return (processChunkData(client_fd));
	}
	else
	{
		if (bytes == 0)
		{
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
			return (REQUEST_DISCONNECT_CLIENT);
		}
		else
		{
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to receive request from Client %s:%d",
						host.c_str(),
						port,
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
			return (SERVER_ERROR);
		}
	}
}

Server::RequestStatus Server::processChunkData(int const &client_fd)
{
	if (clients[client_fd]->getChunkSize() == 0) // if not yet parse the chunk size or the chunk size is 0
	{
		RequestStatus request_status = extractChunkSize(client_fd);
		if (request_status != PARSED_CHUNK_SIZE)
			return (request_status);
	}

	clients[client_fd]->appendToRequestBody(clients[client_fd]->getBodyBuf());
	clients[client_fd]->clearBodyBuf();

	std::vector<std::byte> body = clients[client_fd]->getRequestBody();

	if (body.size() >= clients[client_fd]->getBytesToReceive() + (sizeof(CRLF) - 1)) // last buffer
	{
		std::string final_chunk = "0" CRLF CRLF;
		if (body.size() > final_chunk.length()
			&& std::memcmp(body.data() + body.size() - final_chunk.length(), final_chunk.data(), final_chunk.length()) == 0) // if the chunk contains CRLF 0 CRLF CRLF at the end
		{
			if (body.size() == final_chunk.size() 
				&& std::memcmp(body.data(), final_chunk.data(), final_chunk.length()) == 0)
				return (READY_TO_WRITE);
			RequestStatus request_status = READY_TO_WRITE;
			do {
				clients[client_fd]->appendToRequestBody(clients[client_fd]->getBodyBuf());
				clients[client_fd]->clearBodyBuf();
				body = clients[client_fd]->getRequestBody();
				std::cout << YELLOW << clients[client_fd]->getBytesToReceive() << RESET << std::endl; 
				if (static_cast<char>(body[clients[client_fd]->getBytesToReceive()]) != '\r' || static_cast<char>(body[clients[client_fd]->getBytesToReceive() + 1]) != '\n') // delimiter is not CRLF
					return (BAD_REQUEST);
				std::vector<std::byte> body_buf(body.begin() + clients[client_fd]->getBytesToReceive() + (sizeof(CRLF) - 1), body.end());
				clients[client_fd]->appendToBodyBuf(body_buf);
				clients[client_fd]->resizeRequestBody(clients[client_fd]->getBytesToReceive());
				clients[client_fd]->setChunkSize(0);
				request_status = extractChunkSize(client_fd);
				std::cout << YELLOW << clients[client_fd]->getChunkSize() << RESET << std::endl;
				std::cout << YELLOW << clients[client_fd]->getBytesToReceive() << RESET << std::endl; 
			} while (request_status == PARSED_CHUNK_SIZE);
			return (request_status);
		}
		if (static_cast<char>(body[clients[client_fd]->getBytesToReceive()]) != '\r' || static_cast<char>(body[clients[client_fd]->getBytesToReceive() + 1]) != '\n') // delimiter is not CRLF
			return (BAD_REQUEST);

		std::vector<std::byte> body_buf(body.begin() + clients[client_fd]->getBytesToReceive() + (sizeof(CRLF) - 1), body.end()); // extract the body buffer
		clients[client_fd]->appendToBodyBuf(body_buf);

		clients[client_fd]->resizeRequestBody(clients[client_fd]->getBytesToReceive()); // extract the body
		clients[client_fd]->setChunkSize(0);
		return (BODY_IN_CHUNK);
	}

	return (BODY_IN_CHUNK);
}

Server::RequestStatus Server::extractChunkSize(int const &client_fd)
{
	std::vector<std::byte> body_buf = clients[client_fd]->getBodyBuf();

	std::string final_chunk = "0" CRLF CRLF;
	if (std::memcmp(body_buf.data(), final_chunk.data(), std::min(final_chunk.length(), body_buf.size())) == 0)
	{
		if (body_buf.size() == final_chunk.length()) // if the body buffer is 0 CRLF CRLF
			return (READY_TO_WRITE);
		else if (body_buf.size() > final_chunk.length()) // if the body buffer is more than 0 CRLF CRLF
			return (BAD_REQUEST);
		else
			return (BODY_IN_CHUNK);
	}

	std::vector<std::byte> delimiter = {std::byte('\r'), std::byte('\n')};
	auto delimiter_pos = std::search(body_buf.begin(), body_buf.end(), delimiter.begin(), delimiter.end());
	if (delimiter_pos != body_buf.end()) // if CRLF is found, extract the chunk size
	{
		std::string chunk_size;
		for (auto it = body_buf.begin(); it != delimiter_pos; ++it)
			chunk_size.push_back(static_cast<char>(*it));
		if (chunk_size.length() > 0 && std::all_of(chunk_size.begin(), chunk_size.end(), ::isxdigit)) // if the chunk size only contains hexadecimal character
		{
			clients[client_fd]->setChunkSize(std::stoi(chunk_size, nullptr, 16));

			clients[client_fd]->setBytesToReceive(clients[client_fd]->getBytesToReceive() + clients[client_fd]->getChunkSize());
			clients[client_fd]->eraseBodyBuf(0, std::distance(body_buf.begin(), delimiter_pos) + (sizeof(CRLF) - 1)); // remove the chunk size from body buffer
			return (PARSED_CHUNK_SIZE);
		}
		else // if the chunk size contains non-hexadecimal character
			return (BAD_REQUEST);
	}
	else // if CRLF is not found
	{
		std::string body_buf_str;
		for (auto &ch : body_buf)
			body_buf_str.push_back(static_cast<char>(ch));
		if (!std::all_of(body_buf_str.begin(), body_buf_str.end(), ::isxdigit)) // if the body buffer contains non-hexadecimal character
			return (BAD_REQUEST);
		else
			return (BODY_IN_CHUNK);
	}
}

Server::ResponseStatus Server::sendResponse(int const &client_fd)
{
	//--------------------------------------------------------------
	// TODO - to save the file in Request/Response Class
	// std::ofstream ofs("test.txt");
	// std::vector<std::byte> body = clients[client_fd]->getRequestBody();
	// const std::byte *dataPtr = body.data();
	// std::size_t dataSize = body.size();
	// ofs.write(reinterpret_cast<const char *>(dataPtr), dataSize);
	// ofs.close();
	//--------------------------------------------------------------

	Response response = clients[client_fd]->getResponse();

	std::vector<std::byte>
		full_response = response.formatResponse();

	ssize_t bytes;
	if ((bytes = send(client_fd, &(*(full_response.begin() + clients[client_fd]->getBytesSent())), std::min(full_response.size() - clients[client_fd]->getBytesSent(), static_cast<size_t>(BUFFER_SIZE)), 0)) > 0)
	{
		clients[client_fd]->setBytesSent(clients[client_fd]->getBytesSent() + bytes);
		if (clients[client_fd]->getBytesSent() < full_response.size())
			return (RESPONSE_IN_CHUNK);
		else
		{
			Logger::log(e_log_level::INFO, CLIENT, "Response sent to Client %s:%d - Status: %d",
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)),
						response.getStatusCode());
			if (clients[client_fd]->getConnection() == ConnectionValue::CLOSE || clients[client_fd]->getIsConnectionClose() == true)
				return (RESPONSE_DISCONNECT_CLIENT);
			clients[client_fd]->removeRequest();
			clients[client_fd]->removeResponse();
			return (KEEP_ALIVE); // keep the connection alive by default
		}
	}
	else
	{
		if (bytes == 0)
			Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d disconnected",
						inet_ntoa(getClientIPv4Address(client_fd)),
						ntohs(getClientPortNumber(client_fd)));
		else
			Logger::log(e_log_level::ERROR, SERVER, "Server %s:%d fails to send response to Client %s:%d",
				host.c_str(),
				port,
				inet_ntoa(getClientIPv4Address(client_fd)),
				ntohs(getClientPortNumber(client_fd)));
		return (RESPONSE_DISCONNECT_CLIENT);
	}
}

void Server::createAndSendErrorResponse(HttpStatusCode const &statusCode, int const &client_fd)
{
	clients[client_fd]->createErrorRequest(configs, statusCode);
	clients[client_fd]->createResponse();
	sendResponse(client_fd);
}

int const &Server::getServerFd() const
{
	return (server_fd);
}

std::string const &Server::getHost()
{
	return (host);
}

int const &Server::getPort()
{
	return (port);
}

unsigned short int const &Server::getClientPortNumber(int const &client_fd)
{
	return (clients[client_fd]->getPortNumber());
}

in_addr const &Server::getClientIPv4Address(int const &client_fd)
{
	return (clients[client_fd]->getIPv4Address());
}

void Server::appendConfig(ConfigData const &config)
{
	configs.push_back(config);
}

void Server::removeClient(int const &client_fd)
{
	Logger::log(e_log_level::INFO, CLIENT, "Client %s:%d is removed",
				inet_ntoa(getClientIPv4Address(client_fd)),
				ntohs(getClientPortNumber(client_fd)));
	clients.erase(client_fd);
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

const char *Server::AcceptException::what() const throw()
{
	return "Server::accept() failed";
}
