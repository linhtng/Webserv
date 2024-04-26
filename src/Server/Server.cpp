#include "Server.hpp"
#include "../config_parser/ConfigParser.hpp"
#include <cstring>
#include <fstream>

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
		clients[client_fd].getRequest()->setBodyBuf(request_body_buf);
	}

	if (request_status != BAD_REQUEST)
	// if (request->isBodyExpected() && request_status != BAD_REQUEST)
	{
		request_status = clients[client_fd].getRequest()->getContentLength() ? formRequestBodyWithContentLength(client_fd) : formRequestBodyWithChunk(client_fd); // TODO - check the function for checking 'if the request has content length'
		std::cout << "Content length: " << clients[client_fd].getRequest()->getContentLength() << std::endl;
		// request_status = formRequestBodyWithContentLength(client_fd); // TODO - check the function for checking 'if the request has content length'
		// request_status = formRequestBodyWithChunk(client_fd); // TODO - check the function for checking 'if the request has content length'
		if (request_status == REQUEST_CLIENT_DISCONNECTED || request_status == REQUEST_INTERRUPTED || request_status == BODY_IN_CHUNK)
			return (request_status);
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
			std::cout << "body_length: " << body_length << std::endl;
			std::cout << "delimiter_pos" << delimiter_pos << std::endl;
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
		// std::cout << "request buf: ";
		// for (auto &ch : request->getBodyBuf())
		// 	std::cout << static_cast<char>(ch);
		// std::cout << std::endl;
		request->appendToBody(request->getBodyBuf());
		request->clearBodyBuf();
	}

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		std::string body_buf(buf, buf + bytes);
		appendToBodyString(body_buf, *request);
	}

	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if (errno == EWOULDBLOCK || errno == EAGAIN)
	{
		size_t body_size = clients[client_fd].getRequest()->getBody().size();
		size_t content_length = request->getContentLength();

		std::cout << "body_size: " << body_size << std::endl;
		std::cout << "content_length: " << content_length << std::endl;
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
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	Request *request = clients[client_fd].getRequest();

	// std::cout << "here" << std::endl;

	if (!request->getBodyBuf().empty()) // process any remaining data in the request body buffer
	{
		RequestStatus request_status = processChunkData(client_fd, request->getBodyBuf());
		request->clearBodyBuf();
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_IN_CHUNK)
			return (request_status);
	}

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		std::vector<std::byte> body_buf;
		for (ssize_t i = 0; i < bytes; ++i)
			body_buf.push_back(static_cast<std::byte>(buf[i]));
		std::cout << "body_buf: ";
		for (auto &ch : body_buf)
			std::cout << static_cast<char>(ch);
		std::cout << std::endl;
		RequestStatus request_status = processChunkData(client_fd, body_buf);
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

Server::RequestStatus Server::processChunkData(int const &client_fd, std::vector<std::byte> const &body_buf)
{
	Request *request = clients[client_fd].getRequest();

	// std::cout << "request buf: ";
	// for (auto &ch : body_buf)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;

	if (request->getChunkSize() == 0) // if not yet parse the chunk size or the chunk size is 0
	{
		std::cout << "start extract chunk size" << std::endl;
		RequestStatus request_status = extractChunkSize(client_fd, body_buf);
		std::cout << "request->getChunkSize(): " << request->getChunkSize() << std::endl;
		if (request_status == READY_TO_WRITE || request_status == BAD_REQUEST || request_status == BODY_EXPECTED)
			return (request_status);
	}
	else
		request->appendToBody(body_buf);

	std::vector<std::byte> body = request->getBody();

	// std::cout << "body: ";
	// for (auto &ch : body)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;

	size_t chunk_and_delimiter_size = request->getChunkSize() + (sizeof(CRLF) - 1);
	if (body.size() >= chunk_and_delimiter_size) // last buffer
	{
		std::string expected_sequence = CRLF "0" CRLF CRLF;

		if (std::memcmp(body.data() + body.size() - 7, expected_sequence.data(), expected_sequence.length()) == 0)
		{
			// body.resize(body.size() - expected_sequence.size());
			request->resizeBody(body.size() - expected_sequence.length());

			// std::cout << "body2: ";
			// for (auto &ch : request->getBody())
			// 	std::cout << static_cast<char>(ch);
			// std::cout << std::endl;
			request->appendToBody(body);
			return (READY_TO_WRITE);
		}
		if (static_cast<char>(body[request->getChunkSize()]) != '\r' || static_cast<char>(body[request->getChunkSize() + 1]) != '\n') // delimiter is not CRLF
			return (BAD_REQUEST);

		std::vector<std::byte> body_buf_tmp(body.size() - chunk_and_delimiter_size);
		std::copy(body.begin() + chunk_and_delimiter_size, body.end(), body_buf_tmp.begin());
		request->setBodyBuf(body_buf_tmp);
		std::cout << "buf ";
		for (auto &ch : request->getBodyBuf())
			std::cout << static_cast<char>(ch);
		std::cout << std::endl;
		// body.resize(request->getChunkSize());
		request->resizeBody(request->getChunkSize());
		// std::cout << "body2: ";
		// for (auto &ch : request->getBody())
		// 	std::cout << static_cast<char>(ch);
		// std::cout << std::endl;
		// if (body.size() != request->getChunkSize()) // check chunk size
		// 	return (BAD_REQUEST);
		request->setChunkSize(0);
		return (BODY_IN_CHUNK);
	}
	return (BODY_EXPECTED);
}

Server::RequestStatus Server::extractChunkSize(int const &client_fd, std::vector<std::byte> const &body_buf)
{
	Request *request = clients[client_fd].getRequest();

	std::cout << "buf in extract chunk size: ";
	for (auto &ch : body_buf)
		std::cout << static_cast<char>(ch);
	std::cout << std::endl;

	std::string bodyBufStr;
	for (auto &ch : request->getBodyBuf())
		bodyBufStr.push_back(static_cast<char>(ch));

	std::string final_chunk = "0" CRLF CRLF;

	char final_chunk_array[] = {'0', '\r', '\n', '\r', '\n'};
	// std::vector<std::byte> final_chunk;
	// for (auto ch : final_chunk_array)
	// 	final_chunk.push_back(static_cast<std::byte>(ch));

	std::vector<std::byte> final_chunk_with_CRLF;
	for (int i = 0; i < 3; i++)
		final_chunk_with_CRLF.push_back(static_cast<std::byte>(final_chunk_array[i]));

	std::vector<std::byte> final_chunk_with_CRLF_CR;
	for (int i = 0; i < 4; i++)
		final_chunk_with_CRLF_CR.push_back(static_cast<std::byte>(final_chunk_array[i]));

	std::cout << "after forming chunk" << std::endl;
	// std::cout << std::memcmp(body.data(), final_chunk.data(), 5) << std::endl;
	if (body_buf.size() >= final_chunk.length() && std::memcmp(body_buf.data(), final_chunk.data(), final_chunk.length()) == 0)
	{
		std::cout << "here2" << std::endl;
		return (READY_TO_WRITE);
	}
	std::cout << "after forming chunk 2" << std::endl;

	std::vector<std::byte> delimiter;
	delimiter.push_back(static_cast<std::byte>('\r'));
	delimiter.push_back(static_cast<std::byte>('\n'));

	// std::cout << "body0: ";
	// for (auto &ch : request->getBody())
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;

	// std::cout << "delimiter: ";
	// for (auto &ch : delimiter)
	// 	std::cout << static_cast<char>(ch);
	// std::cout << std::endl;

	auto crlfPos = std::search(body_buf.begin(), body_buf.end(), delimiter.begin(), delimiter.end());

	if (crlfPos != body_buf.end())
	{
		std::cout << "here1" << std::endl;
		std::string hexSize;
		for (auto it = body_buf.begin(); it != crlfPos; ++it)
		{
			hexSize.push_back(static_cast<char>(*it));
			std::cout << static_cast<char>(*it);
		}

		if (std::all_of(hexSize.begin(), hexSize.end(), ::isxdigit))
		{
			int chunkSize = std::stoi(hexSize, nullptr, 16);
			std::cout << "chunk size: " << chunkSize << std::endl;
			request->setChunkSize(chunkSize);
			if (request->getChunkSize() == 0)
				return (std::memcmp(body_buf.data(), final_chunk_with_CRLF.data(), 3) == 0 || std::memcmp(body_buf.data(), final_chunk_with_CRLF_CR.data(), 4) == 0) ? BODY_EXPECTED : BAD_REQUEST;
			// request->eraseBody(request->getBody().size() - body_buf.size(), request->getBody().size() - body_buf.size() + crlfPos - body_buf.begin() + 2);
			std::vector<std::byte> body_buf_tmp;
			for (auto it = crlfPos + 2; it != body_buf.end(); ++it)
				body_buf_tmp.push_back(*it);
			request->appendToBody(body_buf_tmp);
			return PARSED_CHUNK_SIZE;
		}
		else
			return BAD_REQUEST;
	}
	else if (!std::regex_search(bodyBufStr, std::regex("^([0-9A-Fa-f]+)")))
	{
		return BAD_REQUEST;
	}
	else
	{
		std::cout << "here: " << std::endl;
		for (auto &ch : body_buf)
			std::cout << static_cast<char>(ch);
		std::cout << std::endl;
		request->setBodyBuf(body_buf);
		return BODY_EXPECTED;
	}

	// std::smatch match;
	// if (std::regex_search(body, match, std::regex("^([0-9A-Fa-f]+)" CRLF)))
	// {
	// 	request->setChunkSize(std::stoi(match[1], nullptr, 16));
	// 	if (request->getChunkSize() == 0)
	// 		return (body == "0" CRLF || body == "0" CRLF "\r") ? BODY_EXPECTED : BAD_REQUEST;
	// 	// body.erase(0, match[0].length());
	// 	std::cout << "body: ";
	// 	for (auto &ch : request->getBody())
	// 		std::cout << static_cast<char>(ch);
	// 	std::cout << std::endl;
	// 	std::cout << "match[0].length(): " << match[0].length() << std::endl;
	// 	request->eraseBody(0, match[0].length());
	// 	std::cout << "body: ";
	// 	for (auto &ch : request->getBody())
	// 		std::cout << static_cast<char>(ch);
	// 	std::cout << std::endl;
	// 	return (PARSED_CHUNK_SIZE);
	// }
	// else if (!std::regex_search(body, std::regex("^([0-9A-Fa-f]+)"))) // if the chunk is not started with number
	// 	return (BAD_REQUEST);
	// else
	// 	return (BODY_EXPECTED);
}

//--------------------------------------------------------------
// TODO - move to request class
void Server::appendToBodyString(std::string const &str, Request &request)
{
	std::vector<std::byte> new_body_chunk;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		new_body_chunk.push_back(static_cast<std::byte>(*it));
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
	// TODO - to save the file in Request/Response Class
	// std::ofstream ofs("test.txt");
	// std::vector<std::byte> body = clients[client_fd].getRequest()->getBody();
	// const std::byte *dataPtr = body.data();
	// std::size_t dataSize = body.size();
	// ofs.write(reinterpret_cast<const char *>(dataPtr), dataSize);
	// ofs.close();
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

	std::cout << "to get full response" << std::endl;

	std::cout << "client_fd: " << client_fd << std::endl;

	std::cout << "response again: " << std::endl;
	clients[client_fd].getResponse()->printResponseProperties();

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

	// if request header = Connection: close, close connection
	// std::cout << "Response sent from server" << std::endl;
	// return (RESPONSE_CLIENT_DISCONNECTED);
	if (clients[client_fd].getRequest()->getConnection() == ConnectionValue::CLOSE)
	{
		std::cout << "connection close" << std::endl;
		return (RESPONSE_CLIENT_DISCONNECTED);
	}
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

int const &Server::getServerFd() const
{
	return (server_fd);
}

void Server::removeClient(int const &client_fd)
{
	clients.erase(client_fd);
}

std::unordered_map<int, Client> const &Server::getClients() const
{
	return (clients);
}

ConfigData const &Server::getConfig() const
{
	return (config);
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
