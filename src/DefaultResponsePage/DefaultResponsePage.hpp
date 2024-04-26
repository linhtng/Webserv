#ifndef DEFAULT_RESPONSE_HPP
#define DEFAULT_RESPONSE_HPP

#include "../defines.hpp"
#include <vector>
#include "../HttpMessage/HttpMessage.hpp"
#include "../StringUtils/StringUtils.hpp"

namespace DefaultResponsePage
{
	std::vector<std::byte> getResponsePage(HttpStatusCode statusCode);
};

#endif