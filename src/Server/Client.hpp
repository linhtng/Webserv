#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <vector>

class Client
{

private:
	struct sockaddr_in address;
	socklen_t addrlen;
	std::vector<std::byte> request;
	std::vector<std::byte> response;

public:
	Client();
	Client(Client const &src);
	Client &operator=(Client const &rhs);
	~Client();

	struct sockaddr_in &getAndSetAddress(void);
	socklen_t &getAndSetAddrlen(void);
	std::vector<std::byte> getResponse(void) const;

	void setRequest(std::vector<std::byte> new_request_chunk);
	void setResponse(std::vector<std::byte> new_response);
};

#endif