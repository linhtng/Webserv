#include "DirectoryListingPage.hpp"

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

std::vector<std::byte> DirectoryListingPage::getDirectoryListingPage(std::string path, std::string dir)
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

	StringUtils::replaceAll(templateContent, "{{dir_name}}", dir);
	StringUtils::replaceAll(templateContent, "{{list_items}}", listItems);

	std::vector<std::byte> response;
	for (char ch : templateContent)
	{
		response.push_back(static_cast<std::byte>(ch));
	}
	return response;
}
