#include <iostream>
#include <fstream>
#include <filesystem>
#include "ConfigParser.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0];
        std::cout << " <fileName>\n";
        return 0;
    }
    std::string fileName = argv[1];
    try
    {
        ConfigParser parser(fileName);
        parser.extractServerConfigs();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
