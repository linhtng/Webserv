#include "BinaryData.hpp"

std::vector<std::byte> BinaryData::getErrorPage(HttpStatusCode statusCode)
{
	std::string templatePath = DEFAULT_ERROR_TEMPLATE_PATH;
	std::ifstream templateFile(templatePath);
	if (!templateFile.is_open())
	{
		throw std::runtime_error("Error: could not open file " + templatePath);
	}
	std::stringstream buffer;
	buffer << templateFile.rdbuf();
	std::string templateContent = buffer.str();

	std::string statusCodeStr = std::to_string(statusCode);
	std::string statusMessage = DEFAULT_ERROR_MESSAGE;
	try
	{
		statusMessage = HttpUtils::_statusCodeMessages.at(statusCode);
	}
	catch (const std::out_of_range &e)
	{
		Logger::log(INFO, SERVER, "Received unknown error status code: %s", statusCode);
	}
	StringUtils::replaceAll(templateContent, "{{status_code}}", statusCodeStr);
	StringUtils::replaceAll(templateContent, "{{status_message}}", statusMessage);

	std::vector<std::byte> response;
	for (char ch : templateContent)
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	return response;
}

static std::vector<std::string> getDirectoryContents(const std::string &path)
{
	std::vector<std::string> contents;
	DIR *dir = opendir(path.c_str());
	if (dir == nullptr)
	{
		throw std::runtime_error("Error: could not open directory " + path);
	}
	struct dirent *entry;
	while ((entry = readdir(dir)) != nullptr)
	{
		contents.push_back(entry->d_name);
	}
	closedir(dir);
	return contents;
}

std::vector<std::byte> BinaryData::getDirectoryListingPage(std::string locationPath, std::string actualLocationPath, std::string pathAfterLocation)
{
	// get page template
	std::string templatePath = DEFAULT_DIRECTORY_LISTING_TEMPLATE_PATH;
	std::ifstream templateFile(templatePath);
	if (!templateFile.is_open())
	{
		throw std::runtime_error("Error: could not open file " + templatePath);
	}
	std::stringstream buffer;
	buffer << templateFile.rdbuf();
	std::string templateContent = buffer.str();

	// Compose list of contents
	std::string actualDirPath = StringUtils::joinPath(actualLocationPath, pathAfterLocation);
	std::string listItems;
	std::vector<std::string> contents = getDirectoryContents(actualDirPath);
	for (const std::string &content : contents)
	{
		std::string fullPath = "/" + StringUtils::joinPath(locationPath, pathAfterLocation, content);
		listItems += "<li><a href=\"" + fullPath + "\">" + content + "</a></li>\n";
	}

	StringUtils::replaceAll(templateContent, "{{dir_name}}", StringUtils::joinPath(locationPath, pathAfterLocation) + "/");
	StringUtils::replaceAll(templateContent, "{{list_items}}", listItems);

	std::vector<std::byte> response;
	for (char ch : templateContent)
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	return response;
}

std::vector<std::byte> BinaryData::getFileData(std::string path)
{
	path = StringUtils::trimChar(path, '/');
	std::ifstream fileStream(path, std::ios::binary);
	if (!fileStream)
	{
		throw std::runtime_error("Failed to open file: " + path);
	}
	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	std::string fileContent = buffer.str();

	std::vector<std::byte> response;
	for (char ch : fileContent)
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	return response;
}

std::vector<std::byte> BinaryData::strToVectorByte(std::string const &str)
{
	std::vector<std::byte> bytes;
	for (char ch : str)
		bytes.push_back(static_cast<std::byte>(ch));
	return bytes;
}
