#ifndef DEFAULT_ERROR_PAGE_HPP
#define DEFAULT_ERROR_PAGE_HPP

#include "../defines.hpp"
#include "../HttpMessage/HttpMessage.hpp"
#include "../StringUtils/StringUtils.hpp"

#include <vector>
#include <sstream>

namespace DefaultErrorPage
{
	std::vector<std::byte> getErrorPage(HttpStatusCode statusCode);
};

#endif