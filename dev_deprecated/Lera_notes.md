TODO:
- standardise this-> stuff
- check that i didn't mess up with matches array (0th elem is always full string)
+ remove includes of iostream that were for debugging
- add consts where applicable
+ organise the whole routing slash thing
+ figure out boundary shit
- check that asking for upgrade sends proper header back
- remove extra debug couts

POST /hi HTTP/1.1
Host: localhost:10002
Connection: keep-alive
Content-Length: 189
Cache-Control: max-age=0
sec-ch-ua: "Chromium";v="116", "Not)A;Brand";v="24", "Google Chrome";v="116"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "macOS"
Upgrade-Insecure-Requests: 1
Origin: http://localhost:10002
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7ybdh3HK4BBU6x7R
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Site: same-origin
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Referer: http://localhost:10002/hi/uploadPage.html
Accept-Encoding: gzip, deflate, br
Accept-Language: en-US,en;q=0.9

------WebKitFormBoundary7ybdh3HK4BBU6x7R
Content-Disposition: form-data; name="file"; filename="number.txt"
Content-Type: text/plain

12345
------WebKitFormBoundary7ybdh3HK4BBU6x7R--




POST /upload HTTP/1.1
Host: localhost:8081
Connection: keep-alive
Content-Length: 274
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryohNq0PAELV5YFbkJ

------WebKitFormBoundaryohNq0PAELV5YFbkJ
Content-Disposition: form-data; name="fileUpload"; filename="test.sh"
Content-Type: text/x-sh

#!/bin/bash
for i in {1..100}; do
    curl -s -o /dev/null http://localhost:8080/ &
done
------WebKitFormBoundaryohNq0PAELV5YFbkJ--

MULTIFORM:

Each part must contain a Content-Disposition header field where the disposition is form-data. The Content-Disposition header field must also contain an additional parameter of name. For form data that represents the content of a file, a name for the file should be supplied as well, by using a filename parameter of the Content-Disposition header field.
Each part may have an optional Content-Type header field which defaults to text/plain. If the contents of a file are to be sent, the file data should be labeled with an appropriate media type, if known, or application/octet-stream.


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

multipart parsing
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

CGI error page:

<?php
// error.php
$statusCode = $_SERVER['REDIRECT_STATUS'];
$statusMessage = getStatusMessage($statusCode); // Implement this function based on your status code definitions

// Load the template
$template = file_get_contents('error_template.html');

// Replace placeholders with actual values
$template = str_replace('{{status_code}}', $statusCode, $template);
$template = str_replace('{{status_message}}', $statusMessage, $template);

echo $template;
?>

config:

error_page 404 /error.php?status=404;
error_page 500 502 503 504 /error.php?status=500;