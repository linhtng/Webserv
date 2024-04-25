TODO:

POST /upload HTTP/1.1
Host: localhost:8081
Connection: keep-alive
Content-Length: 274
Cache-Control: max-age=0
sec-ch-ua: "Chromium";v="116", "Not)A;Brand";v="24", "Google Chrome";v="116"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "macOS"
Upgrade-Insecure-Requests: 1
Origin: http://localhost:8081
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryohNq0PAELV5YFbkJ
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Site: same-origin
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Referer: http://localhost:8081/upload
Accept-Encoding: gzip, deflate, br
Accept-Language: en-US,en;q=0.9

------WebKitFormBoundaryohNq0PAELV5YFbkJ
Content-Disposition: form-data; name="fileUpload"; filename="test.sh"
Content-Type: text/x-sh

#!/bin/bash
for i in {1..100}; do
    curl -s -o /dev/null http://localhost:8080/ &
done
------WebKitFormBoundaryohNq0PAELV5YFbkJ--

- default error pages (pass messages as params)

{method_token} 

SHOULD MESSAGE BE AN ABSTRACT CLASS THAT REQUEST AND RESPONSE INHERIT FROM? genius or stupid


to parse a message to an encoding in newer versions of cpp use cppcodec:

```
#include <cppcodec/base64_rfc4648.hpp>

std::string convertToUTF8(const std::vector<char>& buffer) {
	return std::string(buffer.begin(), buffer.end());
}
```


how are we gonna read start-line and header in the first place without knowing the encoding though???



"String-based parsers can only be safely used within protocol elements after the element has been extracted from the message, such as within a header field line value after message parsing has delineated the individual field lines."




Error page that has error number and message passed as params
keep error numbers and messages in an unordered_map somewhere? utility class?



Generally, the web server handles basic request/response handling, static file serving, and routing, while the backend application processes dynamic content generation, business logic, and database interactions.


Upon receiving an HTTP/1.1 (or later) request that has a method, target URI, and complete header section that contains a 100-continue expectation and an indication that request content will follow, an origin server MUST send either:

an immediate response with a final status code, if that status can be determined by examining just the method, target URI, and header fields, or
an immediate 100 (Continue) response to encourage the client to send the request content.


Field values containing CR, LF, or NUL characters are invalid and dangerous, due to the varying ways that implementations might parse and interpret those characters; a recipient of CR, LF, or NUL within a field value MUST either reject the message or replace each of those characters with SP before further processing or forwarding of that message.

multiform parsing
content-type form, border
enctype= multipart/form-data

even if request is not chunked, it still may come in several reads

if response is really large, it may not go through on school macs. we should either send in chunks or throw error


chunk size is in HEX
chunks can contain CRLF



HTTP/1.1 does not define any means to limit the size of a chunked response such that an intermediary can be assured of buffering the entire response. Additionally, very large chunk sizes may cause overflows or loss of precision if their values are not represented accurately in a receiving implementation. Therefore, recipients MUST anticipate potentially large hexadecimal numerals and prevent parsing errors due to integer conversion overflows or precision loss due to integer representation.


HTTP/1.1 505 HTTP Version Not Supported
Server: nginx/1.25.4
Date: Thu, 25 Apr 2024 12:35:03 GMT
Content-Type: text/html
Content-Length: 187
Connection: close