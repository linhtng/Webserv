#include "DefaultResponsePage.hpp"

std::vector<std::byte> DefaultResponsePage::getResponsePage(HttpStatusCode statusCode)
{
	// TODO: change path to match directory of the binary
	std::string templatePath = "../pages/errorPage.html";
	std::ifstream templateFile(templatePath);
	if (!templateFile.is_open())
	{
		throw std::runtime_error("Error: could not open file " + templatePath);
	}
	std::stringstream buffer;
	buffer << templateFile.rdbuf();
	std::string templateContent = buffer.str();

	std::string statusCodeStr = std::to_string(statusCode);
	std::string statusMessage = HttpMessage::_statusCodeMessages.at(statusCode);

	StringUtils::replaceAll(templateContent, "{{status_code}}", statusCodeStr);
	StringUtils::replaceAll(templateContent, "{{status_message}}", statusMessage);

	std::vector<std::byte> response;
	for (char ch : templateContent)
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	return response;
}
