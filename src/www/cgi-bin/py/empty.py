
#!/usr/bin/env python3

# while True:
#     pass

# while True:
#     print("Running...")

#CGI_DOCUMENT_RESPONSE,      // Headers + body
# print("Content-Type: text/html")
# print("X-Custom-Header: MyValue")
# print()  # Empty line separates headers from body
# print("<html>")
# print("<body>Hello World from Python!</body>")
# print("</html>")

#CGI_LOCAL_REDIRECT,    // Location: /path
# print("Location: /post/index.html")
# print()  # No body needed for redirect

#CGI_CLIENT_REDIRECT,    // Location: http://...
# print("Location: https://google.com")
# print("Status: 301 Moved Permanently")
# print()

#CGI_NPH_RESPONSE,  // Non-Parsed Headers - Complete HTTP response
# print("HTTP/1.1 200 OK")
# print("Content-Type: application/json")
# print("X-Powered-By: Python-CGI")
# print()
# print('{"status": "success", "message": "NPH response"}')


#CGI_STATUS_RESPONSE,       // Status: 404 Not Found
# print("Status: 418 I'm a teapot")  # RFC 2324
# print("Content-Type: text/html")
# print()
# print("<html><body>I'm a teapot! 🫖</body></html>")

#CGI_PLAIN_TEXT             // No headers, just content
# print("Hello World")
# print("This is plain text without headers")