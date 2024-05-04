#ifndef HTTP_UTILS_HPP
#define HTTP_UTILS_HPP

#include "../defines.hpp"
#include <unordered_map>
#include <string>

namespace HttpUtils
{
	const std::unordered_map<HttpStatusCode, std::string> _statusCodeMessages = {
		{CONTINUE, "Continue"},
		{SWITCHING_PROTOCOLS, "Switching Protocols"},
		{PROCESSING, "Processing"},
		{OK, "OK"},
		{CREATED, "Created"},
		{ACCEPTED, "Accepted"},
		{NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
		{NO_CONTENT, "No Content"},
		{RESET_CONTENT, "Reset Content"},
		{PARTIAL_CONTENT, "Partial Content"},
		{MULTIPLE_CHOICES, "Multiple Choices"},
		{MOVED_PERMANENTLY, "Moved Permanently"},
		{FOUND, "Found"},
		{SEE_OTHER, "See Other"},
		{NOT_MODIFIED, "Not Modified"},
		{TEMPORARY_REDIRECT, "Temporary Redirect"},
		{PERMANENT_REDIRECT, "Permanent Redirect"},
		{BAD_REQUEST, "Bad Request"},
		{UNAUTHORIZED, "Unauthorized"},
		{FORBIDDEN, "Forbidden"},
		{NOT_FOUND, "Not Found"},
		{METHOD_NOT_ALLOWED, "Method Not Allowed"},
		{NOT_ACCEPTABLE, "Not Acceptable"},
		{PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
		{REQUEST_TIMEOUT, "Request Timeout"},
		{CONFLICT, "Conflict"},
		{GONE, "Gone"},
		{LENGTH_REQUIRED, "Length Required"},
		{PRECONDITION_FAILED, "Precondition Failed"},
		{PAYLOAD_TOO_LARGE, "Payload Too Large"},
		{URI_TOO_LONG, "URI Too Long"},
		{UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
		{RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
		{EXPECTATION_FAILED, "Expectation Failed"},
		{IM_A_TEAPOT, "I'm a teapot"},
		{MISDIRECTED_REQUEST, "Misdirected Request"},
		{UPGRADE_REQUIRED, "Upgrade Required"},
		{PRECONDITION_REQUIRED, "Precondition Required"},
		{TOO_MANY_REQUESTS, "Too Many Requests"},
		{REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
		{UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},
		{INTERNAL_SERVER_ERROR, "Internal Server Error"},
		{NOT_IMPLEMENTED, "Not Implemented"},
		{BAD_GATEWAY, "Bad Gateway"},
		{SERVICE_UNAVAILABLE, "Service Unavailable"},
		{GATEWAY_TIMEOUT, "Gateway Timeout"},
		{HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"}};

	const std::unordered_map<std::string, ContentType> _contentTypes = {
		{"text/plain", ContentType::TEXT_PLAIN},
		{"text/html", ContentType::TEXT_HTML},
		{"text/css", ContentType::TEXT_CSS},
		{"text/javascript", ContentType::TEXT_JAVASCRIPT},
		{"application/json", ContentType::APPLICATION_JSON},
		{"application/xml", ContentType::APPLICATION_XML},
		{"application/pdf", ContentType::APPLICATION_PDF},
		{"application/zip", ContentType::APPLICATION_ZIP},
		{"application/octet-stream", ContentType::APPLICATION_OCTET_STREAM},
		{"image/jpeg", ContentType::IMAGE_JPEG},
		{"image/png", ContentType::IMAGE_PNG},
		{"image/gif", ContentType::IMAGE_GIF},
		{"image/svg+xml", ContentType::IMAGE_SVG},
		{"image/webp", ContentType::IMAGE_WEBP},
		{"image/x-icon", ContentType::IMAGE_ICO},
		{"image/bmp", ContentType::IMAGE_BMP},
		{"image/tiff", ContentType::IMAGE_TIFF},
		{"audio/mpeg", ContentType::AUDIO_MPEG},
		{"audio/ogg", ContentType::AUDIO_OGG},
		{"audio/wav", ContentType::AUDIO_WAV},
		{"audio/webm", ContentType::AUDIO_WEBM},
		{"video/mp4", ContentType::VIDEO_MP4},
		{"video/ogg", ContentType::VIDEO_OGG},
		{"video/webm", ContentType::VIDEO_WEBM},
		{"video/mpeg", ContentType::VIDEO_MPEG},
		{"video/quicktime", ContentType::VIDEO_QUICKTIME},
		{"video/x-msvideo", ContentType::VIDEO_AVI},
		{"multipart/form-data", ContentType::MULTIPART_FORM_DATA},
		{"application/x-www-form-urlencoded", ContentType::APPLICATION_X_WWW_FORM_URLENCODED}};

	const std::unordered_map<std::string, HttpMethod> _strToHttpMethod = {
		{"GET", HttpMethod::GET},
		{"HEAD", HttpMethod::HEAD},
		{"POST", HttpMethod::POST},
		{"DELETE", HttpMethod::DELETE}};

	const std::unordered_map<HttpMethod, std::string> _httpMethodToStr =
		{
			{HttpMethod::GET, "GET"},
			{HttpMethod::HEAD, "HEAD"},
			{HttpMethod::POST, "POST"},
			{HttpMethod::DELETE, "DELETE"}};
}

#endif