#include "Server.hpp"
#include <cstring>

Server::Server() : server_fd(0)
{
}

Server::Server(configData_t &config)
	: server_fd(0), config(config)
{
}

Server::Server(Server const &src)
{
	*this = src;
}

Server &Server::operator=(Server const &rhs)
{
	if (this != &rhs)
	{
		server_fd = rhs.server_fd;
		config = rhs.config;
		clients = rhs.clients;
		address = rhs.address;
	}
	return (*this);
}

Server::~Server()
{
}

// set up server socket
void Server::setUpServerSocket()
{
	int opt;

	opt = 1;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create socket file descriptor
		throw SocketCreationException();
	try
	{
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, // set file descriptor to be reuseable
					   sizeof(opt)) < 0)
			throw SocketSetOptionException();
		if (fcntl(server_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
			throw SocketSetNonBlockingException();
		address.sin_family = AF_INET;
		address.sin_port = htons(config.serverPort);
		address.sin_addr.s_addr = inet_addr(config.serverHost.c_str());
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

// accept new connections
std::vector<int> Server::acceptNewConnections()
{
	std::vector<int> client_fds;

	try
	{
		while (true)
		{
			Client client;
			int client_fd = accept(server_fd,
								   (struct sockaddr *)&(client.getAndSetAddress()),
								   &(client.getAndSetAddrlen()));
			if (client_fd < 0)
			{
				if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) // listen() queue is empty or interrupted by a signal
					break;
				else if (errno == ECONNABORTED) // a connection has been aborted
					continue;
				throw AcceptException();
			}
			clients[client_fd] = std::move(client);
			client_fds.push_back(client_fd);
			if (fcntl(client_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0) // set socket to be nonblocking
				throw SocketSetNonBlockingException();
		}
		return (client_fds);
	}
	catch (std::exception &e)
	{
		for (const int fd : client_fds) // close all client fds
			close(fd);
		throw;
	}
}

// receive the request
Server::RequestStatus Server::receiveRequest(int const &client_fd)
{

	Request *request = clients[client_fd].getRequest();

	if (!request)
	{
		std::string request_header;
		std::vector<std::byte> request_body_buf;

		RequestStatus request_header_status = formRequestHeader(client_fd, request_header,
																request_body_buf);
		if (request_header_status == REQUEST_CLIENT_DISCONNECTED || request_header_status == REQUEST_INTERRUPTED)
			return (request_header_status);
		else if (request_header_status == HEADER_NO_DELIMITER)
		{
			clients[client_fd].createRequest(request_header);
			clients[client_fd].createResponse();	  // create response object
			std::cout << "no delimiter in request header" << std::endl; // TODO - set the response with error code
			return (READY_TO_WRITE);
		}

		clients[client_fd].createRequest(request_header); // create request object
		request = clients[client_fd].getRequest();

		//TODO - add check for 'if the request has content length'
		if (request->bodyExpected())
			request->appendToBody(request_body_buf);
		clients[client_fd].setBytesToReceive(request->getContentLength() - request_body_buf.size());

		std::cout << "request_header: " << request_header << std::endl;
	}

	if (request->bodyExpected())
	{
		RequestStatus request_body_status = formRequestBody(client_fd, *request);
		if (request_body_status == REQUEST_CLIENT_DISCONNECTED || request_body_status == REQUEST_INTERRUPTED || request_body_status == BODY_IN_CHUNK)
			return (request_body_status);
		if (request_body_status == MALFORMED_REQUEST)
		{
			clients[client_fd].createResponse();	  // create response object
			std::cout << "body size is larger than content-length" << std::endl;
			return (READY_TO_WRITE);
		}
	}

	clients[client_fd].createResponse(); // create response object
	return (READY_TO_WRITE);
}

// read request header
Server::RequestStatus Server::formRequestHeader(int const &client_fd,
												std::string &request_header, std::vector<std::byte> &request_body_buf)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	std::string delimitor = "\r\n\r\n";
	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		request_header.append(buf, bytes);
		size_t delimitor_pos = request_header.find(delimitor);
		if (delimitor_pos != std::string::npos)
		{
			for (char ch : request_header.substr(delimitor_pos + delimitor.length()))
				request_body_buf.push_back(static_cast<std::byte>(ch));
			request_header.erase(delimitor_pos);
			return (HEADER_DELIMITER_FOUND);
		}
	}
	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if (errno == EWOULDBLOCK || errno == EAGAIN) // if cannot search for the delimiter
		return (HEADER_NO_DELIMITER);
	throw RecvException();
}

// read request body
Server::RequestStatus Server::formRequestBody(int const &client_fd, Request &request)
{
	ssize_t bytes;
	char buf[BUFFER_SIZE];

	while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0)
	{
		//TODO - handle chunk
		//check for hexadecimal and CRLF, save the hexadecimal in chunk_sizes and ignore hexadecimal and CRLF
		//check for 0 and two CRLF, add status of END_OF_CHUNK
		std::vector<std::byte> new_body_chunk;
		for (ssize_t i = 0; i < bytes; ++i)
			new_body_chunk.push_back(static_cast<std::byte>(buf[i]));
		request.appendToBody(new_body_chunk);
		clients[client_fd].setBytesToReceive(clients[client_fd].getBytesToReceive() - bytes); //TODO - only for request with content length
	}
	if (bytes == 0 || errno == ECONNRESET || errno == ETIMEDOUT) // client has shutdown or timeout
		return (REQUEST_CLIENT_DISCONNECTED);
	else if (errno == EINTR) // interrupted by a signal
		return (REQUEST_INTERRUPTED);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN) && clients[client_fd].getBytesToReceive() > 0) // request body send in chunk
		return (BODY_IN_CHUNK);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN) && clients[client_fd].getBytesToReceive() == 0) // read till the end
		return (READY_TO_WRITE);
	else if ((errno == EWOULDBLOCK || errno == EAGAIN) && clients[client_fd].getBytesToReceive() < 0) // body size is larger than content-length
		return (MALFORMED_REQUEST);

	//TODO - handle chunk
	//if the status of not END_OF_CHUNK, check if the hexadecimal matches the actual data size, return BODY_IN_CHUNK
	//if the status is END_OF_CHUNK, return READY_TO_WRITE
	throw RecvException();
}

// send the response
Server::ResponseStatus Server::sendResponse(int const &client_fd)
{
	Response *response = clients[client_fd].getResponse();

	//--------------------------------------------------------------
	// TODO - to combine response header and response body in Response class
	std::vector<std::byte> full_response;
	for (char ch : response->getHeader())
		full_response.push_back(static_cast<std::byte>(ch));
	std::vector<std::byte> body = response->getBody();
	full_response.insert(full_response.end(), body.begin(), body.end());
	//--------------------------------------------------------------

	//--------------------------------------------------------------
	// for printing
	std::cout << "full response: ";
	for (auto &ch : full_response)
		std::cout << static_cast<char>(ch);
	std::cout << std::endl;
	//--------------------------------------------------------------

	ssize_t bytes;
	size_t response_len = full_response.size();
	size_t bytes_sent = 0;
	while (bytes_sent < response_len && (bytes = send(client_fd,
													  &(*(full_response.begin() + bytes_sent)), std::min(response_len - bytes_sent, static_cast<size_t>(BUFFER_SIZE)), 0)) > 0)
		bytes_sent += bytes;

	// if request header = Connection: close, close connection
	// std::cout << "Response sent from server" << std::endl;
	// return (CLOSE_CONNECTION);
	if (bytes_sent >= response_len || errno == EWOULDBLOCK || errno == EAGAIN) // finish sending response
	{
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

Client &Server::getClient(int const &client_fd)
{
	return clients[client_fd];
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
