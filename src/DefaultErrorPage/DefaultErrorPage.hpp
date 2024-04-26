#ifndef DEFAULT_ERROR_HPP
#define DEFAULT_ERROR_HPP

#include "../defines.hpp"
#include <vector>
#include "../HttpMessage/HttpMessage.hpp"
#include "../StringUtils/StringUtils.hpp"

namespace DefaultErrorPage
{
	std::vector<std::byte> getErrorPage(HttpStatusCode statusCode);
};

#endif