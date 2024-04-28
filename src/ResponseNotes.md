# Properties to keep track of

## Properties copied from Request to Response:
- Status Code
- Method
- Target
- Connection

## Properties with default values:
- Content Length (0)
- chunked (false)
- HTTP version major (1)
- HTTP version minor (1) - make sure to check that this is parsed correctly in request

## Important properties in _request
- body
- content type
- content type params

# Headers

## Server (response only)
Always there

## Date
Always there

## Last-Modified
Unnecessary, used for conditional requests

## Content-Type
Needs to indicate retuned type and potentially encoding, needs more work

## Content-Length
Always there, size of body in bytes (set it in one function for all cases alongside other "always there" headers)

## Connection
Always there, copied from request

## Location (response only)
Used for:
- Redirections
- 201 (Created) when the item is at a different location that current URL



# Other properties

## Body
- Error
	- If page specified in config, load that
	- Otherwise, standard error page
- Directory listing
	- Directory listing page
- Resourse (GET/HEAD)
	- Binary content of the file
- File upload
	- Actually, no body is necessary!

# Types of responses

## Redirection

- Location header


## CGI
TODO

## Upload


##


# Methods

## DELETE

### Headers
No special headers

### Status code
- Success
	- 204
- Error
	- 404 not found
	- 403 forbidden
	- 405 method not allowed here

## GET
- Success

## HEAD
Same as GET, but no body

## POST

A simple form using the default application/x-www-form-urlencoded content type:

```
HTTP
Copy to Clipboard
POST /test HTTP/1.1
Host: foo.example
Content-Type: application/x-www-form-urlencoded
Content-Length: 27

field1=value1&field2=value2
```

A form using the multipart/form-data content type:

```
HTTP
Copy to Clipboard
POST /test HTTP/1.1
Host: foo.example
Content-Type: multipart/form-data;boundary="boundary"

--boundary
Content-Disposition: form-data; name="field1"

value1
--boundary
Content-Disposition: form-data; name="field2"; filename="example.txt"

value2
--boundary--
```

- Success
	- 200
	- 201 created (for upload). No body needed, just Lcoation header!




426
Upgrade: HTTP/2.0
Connection: Upgrade

ERROR PAGE EXAMPLE:

		HTTP/1.1 426 Upgrade Required
		Upgrade: HTTP/2.0
		Connection: Upgrade
		Content-Length: 53
		Content-Type: text/plain

		This service requires use of the HTTP/2.0 protocol