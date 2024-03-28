TODO:

- default error pages

{method_token} 




to parse a message to an encoding in newer versions of cpp use cppcodec:

```
#include <cppcodec/base64_rfc4648.hpp>

std::string convertToUTF8(const std::vector<char>& buffer) {
	return std::string(buffer.begin(), buffer.end());
}
```


how are we gonna read start-line and header in the first place without knowing the encoding though???



"String-based parsers can only be safely used within protocol elements after the element has been extracted from the message, such as within a header field line value after message parsing has delineated the individual field lines."
