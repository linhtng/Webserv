#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string.h>
#include <vector>

class Request
{
private:
public:
	// Constructor that takes a string representing start-line and header file lines
	Request(const std::vector<char> &str);
	~Request();

	void setBody(const std::vector<char> &str) const;
	// Also add getters for all the info - or maybe a single getter?
	// no it's a stupid idea because I would need to define a struct for that...
};

#endif
