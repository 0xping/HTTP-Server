#!/usr/bin/env python3

# Set the content type of the response to HTML
print("Content-type: text/html\r\n\r\n", end="")

# Create an HTML page
print("<html>")
print("<head>")
print("<title>Simple CGI Script</title>")
print("</head>")
print("<body>")
print("<h1>Hello, CGI World!</h1>")
print("</body>")
print("</html>")