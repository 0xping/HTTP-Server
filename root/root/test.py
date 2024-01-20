#!/usr/bin/env python
import os
import cgi
import cgitb

# Enable CGI script debugging
cgitb.enable()

# Set Content-Type header
print("Content-Type: text/html")

# Check if the 'my_secret_cookie' already exists in the HTTP_COOKIE
if 'HTTP_COOKIE' in os.environ and 'my_secret_cookie' in os.environ['HTTP_COOKIE']:
	# Cookie already exists, display a message
	print()  # Blank line to indicate the end of headers
	print("<html><body>")
	print("<h1>Cookie already set with value:</h1>")
	print("<p>" + os.environ['HTTP_COOKIE'] + "</p>")
	print("</body></html>")
else:
	# If 'my_secret_cookie' does not exist, check if the form is submitted
	form = cgi.FieldStorage()
	if form.getvalue("secret"):
		# If 'secret' is provided, set the cookie
		secret = form.getvalue("secret")
		print(f"Set-Cookie: my_secret_cookie={secret}; Max-Age=3600")
		print()
		# Display a success message
		print("<html><body>")
		print("<h1>Secret set successfully!</h1>")
		print("</body></html>")
	else:
		# If 'secret' is not provided, display the form
		print()  # Blank line to indicate the end of headers
		print("<html><body>")
		print("<form method='post' action=''>")
		print("<label for='secret'>Enter your secret:</label>")
		print("<input type='text' id='secret' name='secret' required>")
		print("<input type='submit' value='Set Secret'>")
		print("</form>")
		print("</body></html>")
