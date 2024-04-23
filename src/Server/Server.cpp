#include "Server.hpp"
#include "../config_parser/ConfigParser.hpp"
#include <cstring>
#include <fstream>

Server::Server() : server_fd(0)
{
}

Server::Server(ConfigData &config)
	: server_fd(0), config(config)
{
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
		std::cout << "server port: " << config.getServerPort() << std::endl;
		address.sin_port = htons(config.getServerPort());
		std::cout << "server host: " << config.getServerHost() << std::endl;
		address.sin_addr.s_addr = inet_addr(config.getServerHost().c_str());
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

std::vector<int> Server::acceptNewConnections()
{
	std::vector<int> new_client_fds;

	try
	{
		while (true)
		{
			Client client;
			int client_fd = accept(server_fd,
								   (struct sockaddr *)&(client.getAndSetAddress()),
								   &(client.getAndSetAddrlen()));
			std::cout << "client_fd: " << client_fd << std::endl;
			if (client_fd < 0)
			{
				if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) // listen() queue is empty or interrupted by a signal
					break;
				else if (errno == ECONNABORTED) // a connection has been aborted
					continue;
				throw AcceptException();
			}
			clients[client_fd] = std::move(client);
			new_client_fds.push_back(client_fd);
			if (fcntl(client_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
				throw SocketSetNonBlockingException();
		}
		return (new_client_fds);
	}
	catch (std::exception &e)
	{
		for (const int fd : new_client_fds) // close all client fds
			close(fd);
		throw;
	}
}

Server::RequestStatus Server::receiveRequest(int const &client_fd)
{
	RequestStatus request_status = BODY_EXPECTED;

	if (clients[client_fd].isNewRequest()) // if the request is not created yet, create the request with the request header
	{
		std::cout << "new request" << std::endl;
		request_status = createRequestWithHeader(client_fd);
		if (request_status == REQUEST_CLIENT_DISCONNECTED || request_status == REQUEST_INTERRUPTED || request_status == BODY_IN_CHUNK)
			return request_status;
	}

	if (request_status != NO_REQUEST_BODY && request_status != BAD_REQUEST)
	{
		Request *request = clients[client_fd].getRequest();
		if (request->getContentLength()) // TODO - check the function for checking 'if the request has content length'
			request_status = formRequestBodyWithContentLength(client_fd, *request);
		else
			request_status = formRequestBodyWithChunk(client_fd, *request);

		if (request_status == REQUEST_CLIENT_DISCONNECTED || request_status == REQUEST_INTERRUPTED || request_status == BODY_IN_CHUNK || request_status == BODY_IN_PART)
		{
			std::cout << "request_status for body: " << request_status << std::endl;
			return (request_status);
		}
	}

	if (request_status == BAD_REQUEST)
		std::cout << "bad request" << std::endl;

	// Request *request = clients[client_fd].getRequest();
	// std::vector<std::byte> body = request->getBody();
	// std::cout << "-----body-----" << std::endl;
	// for (auto ch : body)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;
	// std::cout << "-----body end-----" << std::endl;

	clients[client_fd].createResponse(); // create response object
	clients[client_fd].setBytesToReceive(0);
	return (READY_TO_WRITE);
}

Server::RequestStatus Server::createRequestWithHeader(int const &client_fd)
{
	std::string request_header;

	RequestStatus request_status = formRequestHeader(client_fd, request_header);
	if (request_status == REQUEST_CLIENT_DISCONNECTED || request_status == REQUEST_INTERRUPTED)
		return (request_status);

	/*
	------------------------------------------------------------------
		Logger - print out the request header and from which client
	------------------------------------------------------------------
	*/

	// std::cout << std::endl;
	// std::cout << "----request_header---- " << std::endl;
	// std::cout << request_header << std::endl;
	// std::cout << "----request_header end---- " << std::endl;
	// std::cout << std::endl;

	clients[client_fd].createRequest(request_header, this->config); // create request object
	Request *request = clients[client_fd].getRequest();

	// if (request->isBodyExpected())
	if (1)
	{
		if (request->getContentLength()) // TODO - check the function for checking 'if the request has content length'
		{
			appendToBodyString(clients[client_fd].getRequestBodyBuf(), *request);
			if (clients[client_fd].getRequestBodyBuf().size() > request->getContentLength())
				return (BAD_REQUEST);
			clients[client_fd].setBytesToReceive(request->getContentLength() - clients[client_fd].getRequestBodyBuf().size());
		}
		return (BODY_EXPECTED);
	}
	return (NO_REQUEST_BODY);
}

Server::RequestStatus Server::formRequestHeader(int const &client_fd, std::string &request_header)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	clients[client_fd].setRequestBodyBuf("");

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		request_header.append(buf, bytes);
		size_t delimiter_pos = request_header.find(CRLF CRLF);
		if (delimiter_pos != std::string::npos)
		{
			clients[client_fd].setRequestBodyBuf(request_header.substr(delimiter_pos + sizeof(CRLF CRLF) - 1));
			request_header.erase(delimiter_pos);
			return (HEADER_DELIMITER_FOUND);
		}
	}
	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if (errno == EWOULDBLOCK || errno == EAGAIN) // if cannot search for the delimiter
		return (BAD_REQUEST);
	throw RecvException();
}

Server::RequestStatus Server::formRequestBodyWithContentLength(int const &client_fd, Request &request)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		if (static_cast<size_t>(bytes) > clients[client_fd].getBytesToReceive())
			return (BAD_REQUEST);
		appendToBodyString(buf, bytes, request);
		clients[client_fd].setBytesToReceive(clients[client_fd].getBytesToReceive() - bytes);
	}
	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN) && clients[client_fd].getBytesToReceive() > 0) // request body send in chunk
		return (BODY_IN_PART);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN) && clients[client_fd].getBytesToReceive() == 0) // read till the end
		return (READY_TO_WRITE);
	throw RecvException();
}

Server::RequestStatus Server::formRequestBodyWithChunk(int const &client_fd, Request &request)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];
	// clients[client_fd].setBytesToReceive(0);
	std::string body;

	std::cout << "formRequestBodyWithChunk" << std::endl;

	if (!clients[client_fd].getRequestBodyBuf().empty())
	{
		std::cout << "request body buf not empty" << std::endl;
		RequestStatus request_status = formRequestBodyWithChunkLoop(client_fd, request, clients[client_fd].getRequestBodyBuf(), body);
		clients[client_fd].setRequestBodyBuf("");
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_IN_CHUNK)
		{
			std::cout << "request_status for request_body_buf: " << request_status << std::endl;
			return (request_status);
		}
	}

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		std::cout << "body buf" << std::endl;
		std::string body_buf(buf, buf + bytes);
		RequestStatus request_status = formRequestBodyWithChunkLoop(client_fd, request, body_buf, body);
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_IN_CHUNK)
		{
			std::cout << "request_status for body_buf: " << request_status << std::endl;
			return (request_status);
		}
	}
	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN)) // no delimiter
	{
		std::cout << "no delimiter in the body" << std::endl;
		return (BODY_IN_CHUNK);
	}
	throw RecvException();
}

Server::RequestStatus Server::formRequestBodyWithChunkLoop(int const &client_fd, Request &request, std::string &body_buf, std::string &body)
{
	body.append(body_buf);
	
	std::cout << "bytes_to_receieve: " << clients[client_fd].getBytesToReceive() << std::endl;
	if (!clients[client_fd].getBytesToReceive()) // if not yet parse the number of bytes for each chunk
	{
		RequestStatus request_status = extractByteNumberFromChunk(body, client_fd);
		std::cout << "extractByteNumberFromChunk: " << clients[client_fd].getBytesToReceive() << std::endl;
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST)
			return (request_status);
		if (request_status == DELIMITER_NOT_FOUND)
		{
			std::cout << "append body here" << std::endl;
			return (request_status);
		}
	}

	if (clients[client_fd].getBytesToReceive() < BUFFER_SIZE) //last buf
	{
		std::cout << "body_buf[clients[client_fd].getBytesToReceive()]: " << body_buf[clients[client_fd].getBytesToReceive()] << std::endl;
		std::cout << "body_buf" << body_buf << std::endl;
		size_t delimiter_pos = body.find(CRLF);
		if (delimiter_pos != std::string::npos) // if find the delimiter
		{
			std::cout << "bytes-to-receive: " << clients[client_fd].getBytesToReceive() << std::endl;
			std::cout << "body_buf.length()" << body_buf.length() << std::endl;
			std::cout << "found CRLF" << std::endl;
			if (std::regex_search(body, std::regex("0" CRLF CRLF "$")))
			{
				std::cout << "read to write" << std::endl;
				body.erase(body.end() - 7, body.end());
				appendToBodyString(body, request);
				return (READY_TO_WRITE);
			}
			clients[client_fd].setRequestBodyBuf(body.substr(delimiter_pos + sizeof(CRLF) - 1));
			std::cout << "request_body_buf: " << clients[client_fd].getRequestBodyBuf() << std::endl;
			body.erase(delimiter_pos);
			// sleep(10);
			appendToBodyString(body, request);
			clients[client_fd].setBytesToReceive(0);
			std::cout << "still in chunk" << std::endl;
			return (BODY_IN_CHUNK);
		}
		else
		{
			std::cout << "no delimiter in the body" << std::endl;
			return (BAD_REQUEST);
		}
	}



	if (body_buf.length() > clients[client_fd].getBytesToReceive())
	{
		std::cout << "body_buf > byte_to_receieve" << std::endl;
		return (BAD_REQUEST);
	}

	clients[client_fd].setBytesToReceive(clients[client_fd].getBytesToReceive() - body_buf.length());
	return (BODY_EXPECTED);
}

Server::RequestStatus Server::extractByteNumberFromChunk(std::string &str, int const &client_fd)
{
	// std::cout << "str: " << str << std::endl;
	std::cout << "first 8 character of request body" << std::endl;
	for (int i = 0; i < 9; i++)
	{
		if (str[i] == '\r')
			std::cout << "r is here" << std::endl;
		else if (str[i] == '\n')
			std::cout << "n is here"
					  << std::endl;
		else
			std::cout << str[i] << std::endl;
	}
	std::cout << std::endl;
	if (str == "0" CRLF CRLF)
		return (READY_TO_WRITE);
	else if (std::regex_search(str, std::regex("^([0-9A-Fa-f]+)" CRLF)))
	{
		std::cout << "ready for extract number" << std::endl;
		size_t number_pos = str.find(CRLF);
		if (std::regex_search(str, std::regex(CRLF "0" CRLF CRLF "$")))
			clients[client_fd].setBytesToReceive(std::stoi(str.substr(0, number_pos), nullptr, 16) + 7);
		else
			clients[client_fd].setBytesToReceive(std::stoi(str.substr(0, number_pos), nullptr, 16) + 2);
		str.erase(0, number_pos + 2);
		return (PARSED_CHUNK_BYTE);
	}
	else if (!std::regex_search(str, std::regex("^([0-9A-Fa-f]+)")))
	{
		std::cout << "cannot extract number" << std::endl;
		return (BAD_REQUEST);
	}

	else
		return (DELIMITER_NOT_FOUND);
}

//--------------------------------------------------------------
// TODO - move to request class
void Server::appendToBodyString(std::string &str, Request &request)
{
	std::vector<std::byte> new_body_chunk;
	for (std::string::iterator it = str.begin(); it != str.end(); ++it)
		new_body_chunk.push_back(static_cast<std::byte>(*it));
	request.appendToBody(new_body_chunk);
}

void Server::appendToBodyString(char buf[BUFFER_SIZE], size_t bytes, Request &request)
{
	std::vector<std::byte> new_body_chunk;
	for (size_t i = 0; i < bytes; ++i)
		new_body_chunk.push_back(static_cast<std::byte>(buf[i]));
	request.appendToBody(new_body_chunk);
}
//--------------------------------------------------------------

Server::ResponseStatus Server::sendResponse(int const &client_fd)
{
	// Response *response = clients[client_fd].getResponse();

	//--------------------------------------------------------------
	// TODO - to combine response header and response body in Response class
	// std::vector<std::byte> full_response;
	// for (char ch : response->getHeader())
	// 	full_response.push_back(static_cast<std::byte>(ch));
	// std::vector<std::byte> body = response->getBody();
	// full_response.insert(full_response.end(), body.begin(), body.end());
	//--------------------------------------------------------------

	//--------------------------------------------------------------
	// TODO - to be done in Request/Response Class

	//--------------------------------------------------------------
	std::ofstream ofs("test.txt");
	std::vector<std::byte> body = clients[client_fd].getRequest()->getBody();
	const std::byte *dataPtr = body.data();
	std::size_t dataSize = body.size();
	ofs.write(reinterpret_cast<const char *>(dataPtr), dataSize);

	ofs.close();

	//--------------------------------------------------------------
	// sample response to be sent to browser
	std::vector<std::byte>
		full_response;
	std::string sample_response = "HTTP/1.1 200 OK\r\n"
								  "Content-Type: text/html\r\n"
								  "Content-Length: 431\r\n"
								  "\r\n"
								  "<!DOCTYPE html>\n"
								  "<html lang=\"en\">\n"
								  "<head>\n"
								  "    <meta charset=\"UTF-8\">\n"
								  "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
								  "    <title>File Upload Example</title>\n"
								  "</head>\n"
								  "<body>\n"
								  "    <h2>Upload a File</h2>\n"
								  "    <form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">\n"
								  "        <input type=\"file\" name=\"fileUpload\" id=\"fileUpload\">\n"
								  "        <button type=\"submit\">Upload</button>\n"
								  "    </form>\n"
								  "</body>\n"
								  "</html>\n";

	for (char ch : sample_response)
		full_response.push_back(static_cast<std::byte>(ch));

	// std::cout << std::endl;
	// std::cout << "-----full response-----" << std::endl;
	// for (auto &ch : full_response)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;
	// std::cout << "-----full response end-----" << std::endl;
	// std::cout << std::endl;

	ssize_t bytes;
	size_t response_len = full_response.size();
	size_t bytes_sent = 0;
	while (bytes_sent < response_len && (bytes = send(client_fd,
													  &(*(full_response.begin() + bytes_sent)), std::min(response_len - bytes_sent, static_cast<size_t>(BUFFER_SIZE)), 0)) > 0)
		bytes_sent += bytes;

	// if request header = Connection: close, close connection
	// std::cout << "Response sent from server" << std::endl;
	// return (RESPONSE_CLIENT_DISCONNECTED);
	if (bytes_sent >= response_len || errno == EWOULDBLOCK || errno == EAGAIN) // finish sending response
	{

		/*
		------------------------------------------------------------------
			Logger - print out the response and to which client
		------------------------------------------------------------------
		*/

		clients[client_fd].removeRequest();
		clients[client_fd].removeResponse();
		std::cout << "Response sent from server" << std::endl;
		return (KEEP_ALIVE); // keep the connection alive by default
	}
	else if (errno == EINTR) // interrupted by a signal
		return (RESPONSE_INTERRUPTED);
	else if (bytes == 0 || errno == ECONNRESET) // client has shutdown or disconnect
		return (RESPONSE_CLIENT_DISCONNECTED);
	throw SendException();
}

int const &Server::getServerFd(void) const
{
	return (server_fd);
}

void Server::removeClient(int const &client_fd)
{
	clients.erase(client_fd);
}

// std::cout << "full response: ";
// for (auto &ch : full_response)
// 	std::cout << static_cast<char>(ch);
// std::cout << std::endl;

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

const char *Server::TimeoutException::what() const throw()
{
	return "Server::Operation timeout";
}

const char *Server::RecvException::what() const throw()
{
	return "Server::recv() failed";
}

const char *Server::SendException::what() const throw()
{
	return "Server::send() failed";
}
