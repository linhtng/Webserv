#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <string>

class Client
{

private:
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
	std::string getResponse(void) const;

	void setResponse(std::string new_response);
};

#endif