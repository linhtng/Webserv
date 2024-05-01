
#include <iostream>
#include <fstream>
#include "CgiHandler.hpp"
#include "../Request/Request.hpp"
#include "../config_parser/ConfigParser.hpp"

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
        std::vector<ConfigData> servers = parser.getServerConfigs();
        // parser.printCluster();
        ConfigData server = servers[0];

        Request request(server, "GET /time.py HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 100\r\nContent-Type: multipart/form-data");
        // request.printRequestProperties();
        // std::cout << std::endl;
        CgiHandler cgiHandler(request, server);
        cgiHandler.executeCgiScript();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
