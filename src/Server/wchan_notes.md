telnet localhost 10002
curl http://localhost:10002
curl --data-binary "@number.txt" http://localhost:10002
curl --data-binary "@number.txt" -H "Transfer-encoding: chunked" http://localhost:10002
curl -F "file=@number.txt" -H "Transfer-encoding: chunked" http://localhost:10002
curl --data-binary "@image.JPG" http://localhost:10002
curl --data-binary "@image.JPG" -H "Transfer-encoding: chunked" http://localhost:10002
curl --resolve webserv3:10002:127.0.0.1 http://localhost:10002/hi


curl --resolve webserv1:10002:127.0.0.1 http://webserv1:10002/hi
curl --resolve webserv2:10002:127.0.0.1 http://webserv2:10002/test
curl --resolve webserv3:10003:127.0.0.1 http://webserv3:10003/bye
curl --resolve webserv4:10003:127.0.0.1 http://webserv4:10003/empty


GET /hi HTTP/1.1
Host: webserv1:10002
Connection: keep-alive

GET /test HTTP/1.1
Host: webserv2:10002
Connection: keep-alive

GET /bye HTTP/1.1
Host: webserv3:10003
Connection: keep-alive

GET /bye HTTP/1.1
Host: webserv4:10003
Connection: keep-alive



