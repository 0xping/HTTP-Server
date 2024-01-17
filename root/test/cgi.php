#!/usr/bin/env php-cgi
<?php
// Set the content type of the response to HTML
echo "Content-type: text/html\r\n\r\n";

// Check if the QUERY_STRING environment variable is set
$queryString = getenv('QUERY_STRING') ?: '';

// Parse the query string into an associative array
parse_str($queryString, $queryParams);

// Extract values from the array or use default values
$name = isset($queryParams['name']) ? $queryParams['name'] : 'Guest';
$id = isset($queryParams['id']) ? $queryParams['id'] : '123';

// Create an HTML page using the query string parameters
echo "<html>\n";
echo "<head>\n";
echo "<title>PHP CGI Script with Query String in Environment Variable</title>\n";
echo "</head>\n";
echo "<body>\n";
echo "<h1>Hello, PHP CGI $name!</h1>\n";
echo "<p>Your ID is: $id</p>\n";
echo "</body>\n";
echo "</html>\n";
?>

