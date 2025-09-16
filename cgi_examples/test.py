#!/usr/bin/env python3

import os

print("Content-type: text/html\r\n\r\n")
print("<html>")
print("<head><title>Python CGI Test</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><b>{key}:</b> {value}</li>")
print("</ul>")
print("</body>")
print("</html>")
