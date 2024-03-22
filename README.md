# Webserv
This project is about writing your own HTTP server.
- Setting up the server for basic reading and writing operations
    - set up the server socket, bind it to a port, and start listening for incoming connections. When a connection is accepted, it should spawn a new thread to handle  the connection 
- Parsing what we exchange with the clients (i.e. messages)
    - Reading the Request from the socket using a function like `recv()`
    - Parsing the Request Line: The first line of the request is the request line, which contains the HTTP method (GET, POST, etc.), the request target (usually a URL or file path), and the HTTP version. For example, parsing the request line `GET /index.html HTTP/1.1`, to extract HTTP method `GET`, the request target `/index.html`, and the HTTP version `HTTP/1.1`

    - Parsing the Headers: After the request line, the request contains several headers, which provide additional information about the request. These are formatted as `Name: Value`. 
    - Parsing the Body: If the request is a POST request, it will have a body after the headers. This will contain the data for the file upload. 
- Implementing logic to handle different HTTP methods, process requests, and generate appropriate responses
    - Handle GET Request with the requested file path from parsing:
        - Locate the file on the disk
        - Read the file into memory
        - Send the file over the connection
    - Handle POST Request with the file data and the destination file path from parsing:
        - Write the file data to the specified file on the disk
        - Send a response confirming the upload
- CGI handling
    - Implement support for executing CGI scripts and returning their output in the HTTP response 
- Configuration files
- Testing:
    - default basic files to test and demonstrate **every** feature works
    - script(s) to check CGI
    - simple website to test the server

# Resources
__Networking__
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/split/)
- [webserv: Building a Non-Blocking Web Server in C++](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7)
- [Socket Programming in C](https://www.geeksforgeeks.org/socket-programming-cc/)
- [Explores epoll](https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642)
- [Using poll() instead of select()](https://www.ibm.com/docs/en/i/7.4?topic=designs-using-poll-instead-select)

__HTTP__
- [How the web works: HTTP and CGI explained](https://www.garshol.priv.no/download/text/http-tut.html)

__General__
- https://github.com/Bima42/webserv
- https://github.com/Kaydooo/Webserv_42

__CGI__
- [C++ Web Programming](https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm)
