#!/usr/bin/python3
import os

print("Content-Type: text/html\r\n\r\n")

print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head><meta charset='UTF-8'><title>CGI Test</title></head>")
print("<body>")
print("<h1>Python CGI Test</h1>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD')}</p>")
print(f"<p>Path: {os.environ.get('SCRIPT_NAME')}</p>")
print("</body>")
print("</html>")
