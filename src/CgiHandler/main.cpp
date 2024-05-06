
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
        parser.printCluster();
        ConfigData server = servers[0];

        // Request request(servers, "GET testQuery.sh?name=Lily&age=18 HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 5\r\nContent-Type: text/html");
        Request request(servers, "POST primeGenerate.py HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 5\r\nContent-Type: text/html");
        // Request request(servers, "POST primeGenerate.py HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36\r\nAccept: */*\r\nConnection: close\r\nContent-Length: 5\r\nContent-Type: text/html");
        std::string body = "100";
        std::vector<std::byte> bodyBytes;
        for (auto c : body)
        {
            bodyBytes.push_back(std::byte(c));
        }
        request.appendToBody(bodyBytes);
        // request.printRequestProperties();
        // std::cout << std::endl;
        std::unordered_map<std::string, std::string> cgiParams;
        // cgiParams["fileName"] = "testQuery.sh";
        cgiParams["fileName"] = "primeGenerate.py";
        cgiParams["queryParams"] = "name=Linh&age=17";
        CgiHandler cgiHandler(request, cgiParams);
        cgiHandler.initializeCgi(request, cgiParams);
        cgiHandler.createCgiProcess();
        std::cout << "Cgi exit status: " << cgiHandler.getCgiExitStatus() << std::endl;
        std::cout << "Cgi output: " << cgiHandler.getCgiOutput() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
