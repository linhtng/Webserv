#include "BinaryData.hpp"

std::vector<std::byte> BinaryData::getErrorPage(HttpStatusCode statusCode)
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

#include <iostream>

static std::vector<std::string> getDirectoryContents(const std::string &path)
{
	std::cout << CYAN "Getting directory contents for path: " << path << ", c_str(): " << path.c_str() << RESET << std::endl;
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

std::vector<std::byte> BinaryData::getDirectoryListingPage(std::string path)
{
	std::string templatePath = "../pages/directoryListing.html";
	std::ifstream templateFile(templatePath);
	if (!templateFile.is_open())
	{
		throw std::runtime_error("Error: could not open file " + templatePath);
	}
	std::stringstream buffer;
	buffer << templateFile.rdbuf();
	std::string templateContent = buffer.str();

	std::string listItems;
	std::vector<std::string> contents = getDirectoryContents(path);
	for (const std::string &content : contents)
	{
		listItems += "<li><a href=\"" + content + "\">" + content + "</a></li>\n";
	}

	StringUtils::replaceAll(templateContent, "{{dir_name}}", path);
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
	std::cout << RED << "got file data, bytes: " << response.size() << RESET << std::endl;
	return response;
}
