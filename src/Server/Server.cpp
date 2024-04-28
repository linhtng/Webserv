#include "Server.hpp"
#include "../config_parser/ConfigParser.hpp"
#include <cstring>
#include <fstream>
#include <array>

Server::Server() : server_fd(-1)
{
}

Server::Server(ConfigData &config)
	: server_fd(-1), config(config)
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
		std::string request_header;
		std::vector<std::byte> request_body_buf;

		request_status = formRequestHeader(client_fd, request_header, request_body_buf);
		if (request_status == REQUEST_CLIENT_DISCONNECTED || request_status == REQUEST_INTERRUPTED)
			return request_status;
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
		clients[client_fd].getRequest()->appendToBodyBuf(request_body_buf);
	}

	if (clients[client_fd].getRequest()->isBodyExpected() && request_status != BAD_REQUEST)
	{
		request_status = clients[client_fd].getRequest()->isChunked()
			? formRequestBodyWithChunk(client_fd)
			: formRequestBodyWithContentLength(client_fd);
		if (request_status == REQUEST_CLIENT_DISCONNECTED || request_status == REQUEST_INTERRUPTED || request_status == BODY_IN_CHUNK)
			return (request_status);
	}

	if (request_status == BAD_REQUEST)
		clients[client_fd].createErrorRequest(config, HttpStatusCode::BAD_REQUEST);

	// Request *request = clients[client_fd].getRequest();
	// std::vector<std::byte> body = request->getBody();
	// std::cout << "-----body-----" << std::endl;
	// for (auto ch : body)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;
	// std::cout << "-----body end-----" << std::endl;

	clients[client_fd].createResponse(); // create response object
	return (READY_TO_WRITE);
}

Server::RequestStatus Server::formRequestHeader(int const &client_fd, std::string &request_header, std::vector<std::byte> &request_body_buf)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		request_header.append(buf, bytes);
		size_t delimiter_pos = request_header.find(CRLF CRLF);
		if (delimiter_pos != std::string::npos)
		{
			size_t body_length = request_header.size() - delimiter_pos - (sizeof(CRLF CRLF) - 1);
			for (size_t i = 0; i < body_length; ++i)
				request_body_buf.push_back(static_cast<std::byte>(request_header[delimiter_pos + (sizeof(CRLF CRLF) - 1) + i]));
			// std::cout << "-----request_body_buf-------" << std::endl;
			// for (auto &ch : request_body_buf)
			// 	std::cout << static_cast<char>(ch);
			// std::cout << "--------------------------" << std::endl;
			std::cout << std::endl;

			request_header.resize(delimiter_pos);
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

Server::RequestStatus Server::formRequestBodyWithContentLength(int const &client_fd)
{
	Request *request = clients[client_fd].getRequest();
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	if (!request->getBodyBuf().empty()) // process any remaining data in the request body buffer
	{
		request->appendToBody(request->getBodyBuf());
		request->clearBodyBuf();
	}

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
		request->appendToBody(buf, bytes);

	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if (errno == EWOULDBLOCK || errno == EAGAIN)
	{
		size_t body_size = request->getBody().size();
		size_t content_length = request->getContentLength();
		if (body_size < content_length) // request body send in chunk
			return (BODY_IN_CHUNK);
		else if (body_size == content_length) // read till the end
			return (READY_TO_WRITE);
		else
			return (BAD_REQUEST);
	}
	throw RecvException();
}

Server::RequestStatus Server::formRequestBodyWithChunk(int const &client_fd)
{
	Request *request = clients[client_fd].getRequest();
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	if (!request->getBodyBuf().empty()) // process any remaining data in the body buffer from request or last chunk
	{
		RequestStatus request_status = processChunkData(client_fd);
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_IN_CHUNK)
			return (request_status);
	}

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		request->appendToBodyBuf(buf, bytes);
		RequestStatus request_status = processChunkData(client_fd);
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_IN_CHUNK)
			return (request_status);
	}
	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN)) // no delimiter
		return (BODY_IN_CHUNK);

	throw RecvException();
}

Server::RequestStatus Server::processChunkData(int const &client_fd)
{
	Request *request = clients[client_fd].getRequest();
	// std::cout << "buf: ";
	// for (auto &ch : request->getBodyBuf())
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;

	if (request->getChunkSize() == 0) // if not yet parse the chunk size or the chunk size is 0
	{
		RequestStatus request_status = extractChunkSize(client_fd);
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_EXPECTED)
			return (request_status);
	}
	request->appendToBody(request->getBodyBuf());
	request->clearBodyBuf();

	std::vector<std::byte> body = request->getBody();
	size_t total_body_size = request->getBytesToReceive() + (sizeof(CRLF) - 1);
	if (body.size() >= total_body_size) // last buffer
	{
		std::string final_chunk_ending = CRLF "0" CRLF CRLF;
		if (body.size() > final_chunk_ending.length()
			&& std::memcmp(body.data() + body.size() - final_chunk_ending.length(), final_chunk_ending.data(), final_chunk_ending.length()) == 0) //if the chunk contains CRLF 0 CRLF CRLF at the end
		{
			request->resizeBody(body.size() - final_chunk_ending.length());
			return (READY_TO_WRITE);
		}
		if (static_cast<char>(body[request->getBytesToReceive()]) != '\r' || static_cast<char>(body[request->getBytesToReceive() + 1]) != '\n') // delimiter is not CRLF
			return (BAD_REQUEST);

		std::vector<std::byte> body_buf(body.begin() + total_body_size, body.end()); //extract the body buffer
		request->appendToBodyBuf(body_buf);

		request->resizeBody(request->getBytesToReceive()); //extrac the body
		request->setChunkSize(0);
		return (BODY_IN_CHUNK);
	}

	return (BODY_EXPECTED);
}

Server::RequestStatus Server::extractChunkSize(int const &client_fd)
{
	Request *request = clients[client_fd].getRequest();
	std::vector<std::byte> body_buf = request->getBodyBuf();

	std::string final_chunk = "0" CRLF CRLF;
	if (std::memcmp(body_buf.data(), final_chunk.data(), final_chunk.length()) == 0)
	{
		if (body_buf.size() == final_chunk.length()) // if the body buffer is 0 CRLF CRLF
			return (READY_TO_WRITE);
		else if (body_buf.size() > final_chunk.length()) // if the body buffer is more than 0 CRLF CRLF
			return (BAD_REQUEST);
		else
			return (BODY_EXPECTED);
	}

	std::vector<std::byte> delimiter = {std::byte('\r'), std::byte('\n')};
	auto delimiter_pos = std::search(body_buf.begin(), body_buf.end(), delimiter.begin(), delimiter.end());
	if (delimiter_pos != body_buf.end()) // if CRLF is found, extract the chunk size
	{
		std::string chunk_size;
		for (auto it = body_buf.begin(); it != delimiter_pos; ++it)
			chunk_size.push_back(static_cast<char>(*it));
		if (std::all_of(chunk_size.begin(), chunk_size.end(), ::isxdigit)) // if the chunk size only contains hexadecimal character
		{
			request->setChunkSize(std::stoi(chunk_size, nullptr, 16));

			request->setBytesToReceive(request->getBytesToReceive() + request->getChunkSize());
			request->eraseBodyBuf(0, std::distance(body_buf.begin(), delimiter_pos) + (sizeof(CRLF) - 1)); //remove the chunk size from body buffer
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
			return (BODY_EXPECTED);
	}
}

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
	// TODO - to save the file in Request/Response Class
	std::ofstream ofs("test.txt");
	std::vector<std::byte> body = clients[client_fd].getRequest()->getBody();
	const std::byte *dataPtr = body.data();
	std::size_t dataSize = body.size();
	ofs.write(reinterpret_cast<const char *>(dataPtr), dataSize);
	ofs.close();
	//--------------------------------------------------------------

	// sample response to be sent to browser
	// std::vector<std::byte>
	// 	full_response;
	// std::string sample_response = "HTTP/1.1 200 OK\r\n"
	// 							  "Content-Type: text/html\r\n"
	// 							  "Content-Length: 431\r\n"
	// 							  "\r\n"
	// 							  "<!DOCTYPE html>\n"
	// 							  "<html lang=\"en\">\n"
	// 							  "<head>\n"
	// 							  "    <meta charset=\"UTF-8\">\n"
	// 							  "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
	// 							  "    <title>File Upload Example</title>\n"
	// 							  "</head>\n"
	// 							  "<body>\n"
	// 							  "    <h2>Upload a File</h2>\n"
	// 							  "    <form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">\n"
	// 							  "        <input type=\"file\" name=\"fileUpload\" id=\"fileUpload\">\n"
	// 							  "        <button type=\"submit\">Upload</button>\n"
	// 							  "    </form>\n"
	// 							  "</body>\n"
	// 							  "</html>\n";

	// for (char ch : sample_response)
	// 	full_response.push_back(static_cast<std::byte>(ch));

	// std::cout << std::endl;
	// std::cout << "-----full response-----" << std::endl;
	// for (auto &ch : full_response)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;
	// std::cout << "-----full response end-----" << std::endl;
	// std::cout << std::endl;

	std::vector<std::byte>
		full_response = clients[client_fd].getResponse()->formatResponse();

	std::cout << std::endl;
	std::cout << "-----full response-----" << std::endl;
	for (auto &ch : full_response)
		std::cout << static_cast<char>(ch);
	std::cout << std::endl;
	std::cout << "-----full response end-----" << std::endl;
	std::cout << std::endl;

	ssize_t bytes;
	size_t response_len = full_response.size();
	size_t bytes_sent = 0;
	while (bytes_sent < response_len && (bytes = send(client_fd,
													  &(*(full_response.begin() + bytes_sent)), std::min(response_len - bytes_sent, static_cast<size_t>(BUFFER_SIZE)), 0)) > 0)
		bytes_sent += bytes;

	if (clients[client_fd].getRequest()->getConnection() == ConnectionValue::CLOSE)
		return (RESPONSE_CLIENT_DISCONNECTED);
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

void Server::createAndSendErrorResponse(HttpStatusCode const &statusCode, int const &client_fd)
{
	clients[client_fd].createErrorRequest(config, statusCode);
	clients[client_fd].createResponse();
	sendResponse(client_fd);
}

int const &Server::getServerFd() const
{
	return (server_fd);
}

void Server::removeClient(int const &client_fd)
{
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