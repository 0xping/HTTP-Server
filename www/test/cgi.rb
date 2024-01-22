#!/usr/bin/env ruby

require 'cgi'
require 'uri'

# Set the content type of the response to HTML
puts "Content-type: text/html\r\n\r\n"

# Check if the QUERY_STRING environment variable is set
query_string = ENV['QUERY_STRING'] || ''

# Parse the query string into an associative hash
query_params = CGI.parse(query_string)

# Extract values from the hash or use default values
name = query_params['name']&.first || 'Guest'
id = query_params['id']&.first || '123'

# Create an HTML page using the query string parameters
puts "<html>\n"
puts "<head>\n"
puts "<title>Ruby CGI Script with Query String in Environment Variable</title>\n"
puts "</head>\n"
puts "<body>\n"
puts "<h1>Hello, Ruby CGI #{name}!</h1>\n"
puts "<p>Your ID is: #{id}</p>\n"
puts "</body>\n"
puts "</html>\n"
