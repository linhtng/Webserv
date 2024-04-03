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
	POST
};

enum HttpErrorCode
{
	OK = 200,
	CREATED = 201,
	ACCEPTED = 202,
	NO_CONTENT = 204,
	MULTIPLE_CHOICES = 300,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	SEE_OTHER = 303,
	NOT_MODIFIED = 304,
	USE_PROXY = 305,
	TEMPORARY_REDIRECT = 307,
	PERMANENT_REDIRECT = 308,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	PAYMENT_REQUIRED = 402,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	NOT_ACCEPTABLE = 406,
	PROXY_AUTHENTICATION_REQUIRED = 407,
	REQUEST_TIMEOUT = 408,
	CONFLICT = 409,
	GONE = 410,
	LENGTH_REQUIRED = 411,
	PRECONDITION_FAILED = 412,
	CONTENT_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	UNSUPPORTED_MEDIA_TYPE = 415,
	RANGE_NOT_SATISFIABLE = 416,
	EXPECTATION_FAILED = 417,
	IM_A_TEAPOT = 418,			 //
	MISDIRECTED_REQUEST = 421,	 // probably not needed
	UNPROCESSABLE_CONTENT = 422, // syntactically correct but semantically incorrect, very rare, 400 is used instead
	// 422 response is more typically associated with a POST that accepts data in a specific format in the body of the request.
	UPGRADE_REQUIRED = 426, // HTTP protocol update required
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502, // probably not needed
	SERVICE_UNAVAILABLE = 503,
	GATEWAY_TIMEOUT = 504, // probably not needed
	HTTP_VERSION_NOT_SUPPORTED = 505
};

#endif
