#ifndef DEFINES_HPP
#define DEFINES_HPP

#define CR "\r"
#define LF "\n"
#define CRLF "\r\n"
#define SP " "
#define DEFAULT_TIMEOUT 1000

enum RequestStatus
{
	SUCCESS,
	ERROR
};

enum Method
{
	GET,
	HEAD,
	POST
};

enum HttpStatusCode
{
	UNDEFINED = 0,
	CONTINUE = 100,			   // response to Expect: 100-continue in the headers
	SWITCHING_PROTOCOLS = 101, // only needed for protocol upgrades
	PROCESSING = 102,		   // deprecated
	EARLY_HINTS = 103,		   // primarily intended for use with the Link header, allows the browser to pre-connect to resources
	OK = 200,
	CREATED = 201,						 // response to POST, resource created
	ACCEPTED = 202,						 // intended for cases where another process or server handles the request, or for batch processing
	NON_AUTHORITATIVE_INFORMATION = 203, // proxy stuff
	NO_CONTENT = 204,					 // request has succeeded, but the client doesn't need to navigate away from its current page
	RESET_CONTENT = 205,				 // clear form, refresh, etc.
	PARTIAL_CONTENT = 206,				 // used with Range header, streaming, downloads, etc.
	MULTI_STATUS = 207,					 // WebDAV, probably not needed
	ALREADY_REPORTED = 208,				 // related to 207
	IM_USED = 226,						 // browsers don't support this, some fancy stuff
	MULTIPLE_CHOICES = 300,				 // rarely used, generate Location header if there's a preferred location
	MOVED_PERMANENTLY = 301,			 // redirect, put new url to Location header. used for GET and HEAD
	FOUND = 302,						 // redirect, but temporary, which is used by SEO. used for GET and HEAD
	SEE_OTHER = 303,					 // redirect to another resource, method used to display this redirected page is always GET
	NOT_MODIFIED = 304,					 // used when request is a conditional GET or HEAD request with an If-None-Match or an If-Modified-Since header and the condition evaluates to false
	TEMPORARY_REDIRECT = 307,			 // 302 but for POST
	PERMANENT_REDIRECT = 308,			 // 301 but for POST
	BAD_REQUEST = 400,					 // malformed request syntax, invalid request message framing, or deceptive request routing
	UNAUTHORIZED = 401,
	FORBIDDEN = 403, // credentials are valid, but they do not have the necessary permissions. or accessing HTTPS-only resources over HTTP
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405, // MUST generate an Allow header field containing a list of the target resource's currently supported methods if the requested method is known by the origin server but not supported by the target resource. "I know what you want but I don't allow that"
	NOT_ACCEPTABLE = 406,
	PROXY_AUTHENTICATION_REQUIRED = 407, // proxy stuff, probably not needed
	REQUEST_TIMEOUT = 408,				 // server did not receive a complete request message within the time that it was prepared to wait
	CONFLICT = 409,						 // for example, trying to delete a resource that is already deleted or trying to create a resource with a unique identifier that already exists could result in a conflict.
	GONE = 410,							 // preferred over 404 if the resource is permanently gone
	LENGTH_REQUIRED = 411,				 // when Content-Length is missing for POST or PUT
	PRECONDITION_FAILED = 412,			 // at least one condition in If-Match header failed
	CONTENT_TOO_LARGE = 413,			 // we should terminate the request or close the connection. if temporaruy, generate a Retry-After header
	URI_TOO_LONG = 414,					 // rare
	UNSUPPORTED_MEDIA_TYPE = 415,		 // occurs due to the request's indicated Content-Type or Content-Encoding, or as a result of inspecting the data directly. Generate Accept-Encoding or Accept header in a response
	RANGE_NOT_SATISFIABLE = 416,		 // used with Range header. Generate Content-Range header in response. BUT! Because servers are free to ignore Range, many implementations will respond with the entire selected representation in a 200 (OK) response
	EXPECTATION_FAILED = 417,			 // Expect header is not supported or the server cannot meet the expectation. A server that receives an Expect field value containing a member other than 100-continue MAY respond with a 417
	IM_A_TEAPOT = 418,					 //
	MISDIRECTED_REQUEST = 421,			 // probably not needed
	UNPROCESSABLE_CONTENT = 422,		 // syntactically correct but semantically incorrect, very rare, 400 is used instead
	// 422 response is more typically associated with a POST that accepts data in a specific format in the body of the request.
	UPGRADE_REQUIRED = 426, // HTTP protocol update required
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,			 // server does not recognize the request method. "I don't understand what you want" - this is probably what we need!
	BAD_GATEWAY = 502,				 // probably not needed
	SERVICE_UNAVAILABLE = 503,		 // Note: The existence of the 503 status code does not imply that a server has to use it when becoming overloaded. Some servers might simply refuse the connection. MAY send a Retry-After header with the response
	GATEWAY_TIMEOUT = 504,			 // probably not needed
	HTTP_VERSION_NOT_SUPPORTED = 505 // for major versions upwards of 1
};

#endif
