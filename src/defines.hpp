#ifndef DEFINES_HPP
#define DEFINES_HPP

#define SERVER_SOFTWARE "webservant v0.1"
#define DEFAULT_ERROR_MESSAGE "An error occurred"
#define CRITICAL_ERROR_RESPONSE "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
// TODO: change path to match from directory of the binary
#define DEFAULT_ERROR_TEMPLATE_PATH "../pages/errorPage.html"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define BLACK "\033[1;30m"
#define WHITE "\033[1;37m"
#define MAGENTA "\033[1;35m"
#define RESET "\e[0m"

#define CR "\r"
#define LF "\n"
#define NUL "\0"
#define CRLF "\r\n"
#define SP " "
#define HTAB "\t"
#define VCHAR_REGEX "[[:print:]]"
#define DIGIT_REGEX "[0-9]"
#define ALPHA_REGEX "[A-Za-z]"
#define RWS_REGEX "[\t ]+"
#define URL_REGEX "[a-zA-Z0-9-._~:/?#@!$&'()*+,;=%]+"

#define IMPLEMENTED_HTTP_METHODS_REGEX "(GET|HEAD|POST|DELETE)"
#define REQUEST_LINE_REGEX "^" \
						   "(" ALPHA_REGEX "+)" SP "(.+)" SP "HTTP/(\\d{1,3})\\.(\\d{1,3})?$"
#define HOST_REGEX "([^:]+):(\\d+)"

#define GATEWAY_INTERFACE "CGI/1.1"
#define SERVER_PROTOCOL "HTTP/1.1"
#define READ_END 0
#define WRITE_END 1
#define CGI_OUTPUT_BUFFER_SIZE 1024
#define CGI_TIMEOUT 2
#define CGI_EXIT_SUCCESS 0

#define BACKLOG 512
#define SERVER_BUFFER_SIZE 100000
#define MAX_REQUEST_HEADER_LENGTH 8192

#define SERVER_TIMEOUT 3000

enum ConnectionValue
{
	KEEP_ALIVE,
	CLOSE,
	UPGRADE
};

enum HttpMethod
{
	UNDEFINED_METHOD,
	GET,
	HEAD,
	POST,
	DELETE,
};

// not used
enum ContentType
{
	UNDEFINED_CONTENT_TYPE,
	TEXT_PLAIN,
	TEXT_HTML,
	TEXT_CSS,
	TEXT_JAVASCRIPT,
	APPLICATION_JSON,
	APPLICATION_XML,
	APPLICATION_PDF,
	APPLICATION_ZIP,
	APPLICATION_OCTET_STREAM,
	IMAGE_JPEG,
	IMAGE_PNG,
	IMAGE_GIF,
	IMAGE_SVG,
	IMAGE_WEBP,
	IMAGE_ICO,
	IMAGE_BMP,
	IMAGE_TIFF,
	AUDIO_MPEG,
	AUDIO_OGG,
	AUDIO_WAV,
	AUDIO_WEBM,
	VIDEO_MP4,
	VIDEO_OGG,
	VIDEO_WEBM,
	VIDEO_MPEG,
	VIDEO_QUICKTIME,
	VIDEO_AVI,
	MULTIPART_FORM_DATA,
	APPLICATION_X_WWW_FORM_URLENCODED
};

#define VALID_HTTP_METHODS                      \
	{                                             \
		"GET", "HEAD", "POST", "DELETE", "OPTIONS", \
				"PUT", "PATCH", "TRACE", "CONNECT"      \
	}

#define IMPLEMENTED_HTTP_METHODS    \
	{                                 \
		"GET", "HEAD", "POST", "DELETE" \
	}

#define VALID_CGI_EXTEN \
	{                     \
		".py", ".sh"        \
	}

enum HttpStatusCode
{
	UNDEFINED_STATUS = 0,
	CONTINUE = 100,						 // response to Expect: 100-continue in the headers
	SWITCHING_PROTOCOLS = 101, // only needed for protocol upgrades
	PROCESSING = 102,					 // deprecated
	EARLY_HINTS = 103,				 // primarily intended for use with the Link header, allows the browser to pre-connect to resources
	OK = 200,
	CREATED = 201,											 // response to POST, resource created
	ACCEPTED = 202,											 // intended for cases where another process or server handles the request, or for batch processing
	NON_AUTHORITATIVE_INFORMATION = 203, // proxy stuff
	NO_CONTENT = 204,										 // request has succeeded, but the client doesn't need to navigate away from its current page
	RESET_CONTENT = 205,								 // clear form, refresh, etc.
	PARTIAL_CONTENT = 206,							 // used with Range header, streaming, downloads, etc.
	MULTI_STATUS = 207,									 // WebDAV, probably not needed
	ALREADY_REPORTED = 208,							 // related to 207
	IM_USED = 226,											 // browsers don't support this, some fancy stuff
	MULTIPLE_CHOICES = 300,							 // rarely used, generate Location header if there's a preferred location
	MOVED_PERMANENTLY = 301,						 // redirect, put new url to Location header. used for GET and HEAD
	FOUND = 302,												 // redirect, but temporary, which is used by SEO. used for GET and HEAD
	SEE_OTHER = 303,										 // redirect to another resource, method used to display this redirected page is always GET
	NOT_MODIFIED = 304,									 // used when request is a conditional GET or HEAD request with an If-None-Match or an If-Modified-Since header and the condition evaluates to false
	TEMPORARY_REDIRECT = 307,						 // 302 but for POST
	PERMANENT_REDIRECT = 308,						 // 301 but for POST
	BAD_REQUEST = 400,									 // malformed request syntax, invalid request message framing, or deceptive request routing
	UNAUTHORIZED = 401,
	FORBIDDEN = 403, // credentials are valid, but they do not have the necessary permissions. or accessing HTTPS-only resources over HTTP
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405, // MUST generate an Allow header field containing a list of the target resource's currently supported methods if the requested method is known by the origin server but not supported by the target resource. "I know what you want but I don't allow that"
	NOT_ACCEPTABLE = 406,
	PROXY_AUTHENTICATION_REQUIRED = 407, // proxy stuff, probably not needed
	REQUEST_TIMEOUT = 408,							 // server did not receive a complete request message within the time that it was prepared to wait
	CONFLICT = 409,											 // for example, trying to delete a resource that is already deleted or trying to create a resource with a unique identifier that already exists could result in a conflict.
	GONE = 410,													 // preferred over 404 if the resource is permanently gone
	LENGTH_REQUIRED = 411,							 // when Content-Length is missing for POST or PUT
	PRECONDITION_FAILED = 412,					 // at least one condition in If-Match header failed
	PAYLOAD_TOO_LARGE = 413,						 // we should terminate the request or close the connection. if temporaruy, generate a Retry-After header
	URI_TOO_LONG = 414,									 // rare
	UNSUPPORTED_MEDIA_TYPE = 415,				 // occurs due to the request's indicated Content-Type or Content-Encoding, or as a result of inspecting the data directly. Generate Accept-Encoding or Accept header in a response
	RANGE_NOT_SATISFIABLE = 416,				 // used with Range header. Generate Content-Range header in response. BUT! Because servers are free to ignore Range, many implementations will respond with the entire selected representation in a 200 (OK) response
	EXPECTATION_FAILED = 417,						 // Expect header is not supported or the server cannot meet the expectation. A server that receives an Expect field value containing a member other than 100-continue MAY respond with a 417
	IM_A_TEAPOT = 418,									 //
	MISDIRECTED_REQUEST = 421,					 // for wrong host/port
	UNPROCESSABLE_CONTENT = 422,				 // syntactically correct but semantically incorrect, very rare, 400 is used instead
	// 422 response is more typically associated with a POST that accepts data in a specific format in the body of the request.
	UPGRADE_REQUIRED = 426, // HTTP protocol update required
	PRECONDITION_REQUIRED = 428,
	TOO_MANY_REQUESTS = 429,
	REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
	UNAVAILABLE_FOR_LEGAL_REASONS = 451,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,					 // server does not recognize the request method. "I don't understand what you want" - this is probably what we need!
	BAD_GATEWAY = 502,							 // probably not needed
	SERVICE_UNAVAILABLE = 503,			 // Note: The existence of the 503 status code does not imply that a server has to use it when becoming overloaded. Some servers might simply refuse the connection. MAY send a Retry-After header with the response
	GATEWAY_TIMEOUT = 504,					 // probably not needed
	HTTP_VERSION_NOT_SUPPORTED = 505 // for major versions upwards of 1
};

#endif
