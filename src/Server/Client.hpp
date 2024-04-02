#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <string>

class Client
{

private:
	int client_fd;
	struct sockaddr_in address;
	socklen_t addrlen;
	std::string response;

public:
	Client();
	Client(Client const &src);
	Client &operator=(Client const &rhs);
	~Client();

	struct sockaddr_in &getAndSetAddress(void);
	socklen_t &getAndSetAddrlen(void);
	const int &getClientFd(void) const;
	std::string getResponse(void) const;

	void setFd(int client_fd);
	void setResponse(std::string new_response);
};

#endif