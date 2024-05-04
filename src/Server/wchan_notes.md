telnet localhost 10002
curl http://localhost:10002/hi
curl --data-binary "@number.txt" http://localhost:10002
curl --data-binary "@number.txt" -H "Transfer-encoding: chunked" http://localhost:10002
curl -F "file=@number.txt" -H "Transfer-encoding: chunked" http://localhost:10002
curl --data-binary "@image.JPG" http://localhost:10002
curl --data-binary "@image.JPG" -H "Transfer-encoding: chunked" http://localhost:10002

curl -X UNKNOWN http://localhost:10002



- Content-Length = 0, isBodyExpected() true or false; => check requeest header function
- Post method with no body? => check requeest header function
- GetMethod -> return string
- Remove extra empty line in saving file

POST /hi HTTP/1.1
Host: localhost:10002
Connection: keep-alive
Content-Length: 5

12345


POST /hi HTTP/1.1
Host: localhost:10002
Connection: close

POST /hi HTTP/1.1
Host: localhost:10002
Connection: keep-alive



