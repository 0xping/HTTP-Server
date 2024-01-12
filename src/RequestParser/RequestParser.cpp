#include "RequestParser.hpp"

RequestParser::RequestParser()
{
	this->headersLoaded = false;
	this->isCGIfile = false;
}


int RequestParser::loadHeaders(Binary &data)
{
	std::cout << "Loading HTTP Headers \n\n\n";
	// remove newlines in the beginning
	while(data.toStr().length() && data.toStr()[0] == '\n')
		data.erase(0,1);

	// Check for the end of HTTP headers (double newline or double \r\n)
	size_t headerEnd = std::min(data.toStr().find("\r\n\r\n"), data.toStr().find("\n\n"));
	if (headerEnd != std::string::npos) {
		// Headers received, process the headers
		std::string headersStr = data.toStr().substr(0, headerEnd);
		// TODO: Trim the data first || NOT NEEDED
		std::vector<std::string> delimiters;
		delimiters.push_back("\n\n");
		delimiters.push_back("\r\n");

		std::vector<std::string> lines = splitWithDelimiters(headersStr, delimiters);
		for (size_t i = 0; i < lines.size(); i++) {
			if (i == 0) {
				std::vector<std::string> words = strSplit(lines[i], " ");
				if (words.size() != 3|| words[2] != "HTTP/1.1")
					throw HttpError(BadRequest, "Bad Request");
				message.method = words[0];
				message.uri.fullUri = words[1];
			} else {
				size_t colonPos = lines[i].find(':');
				std::string key = lines[i].substr(0, colonPos);
				std::string value = (colonPos != std::string::npos) ? lines[i].substr(colonPos + 1) : "";
				message.headers[key] = value;
			}
		}
		this->headersLoaded = true;
		size_t bodyStart = headerEnd == data.find("\r\n\r\n") ? headerEnd + 2 : headerEnd + 4;
		data = data.substr(bodyStart);
	}
	return 0;
}

void RequestParser::parseRequest()
{
	std::map<std::string, std::string>::iterator TransferEncoding = message.headers.find("Transfer-Encoding");
	std::map<std::string, std::string>::iterator contentLength = message.headers.find("Content-Length");

	if (message.method != "GET" && message.method != "POST" && message.method != "DELETE")
		throw HttpError(NotImplemented, "Not Implemented");

	if (parseUri(message.uri.fullUri)) // returns true if location has a redirect
		return ;

	if (TransferEncoding != message.headers.end())
	{
		if (TransferEncoding->second != "chunked")
			throw HttpError(NotImplemented, "Not Implemented");
	}
	else if (contentLength == message.headers.end() && message.method == "POST")
		throw HttpError(BadRequest, "Bad Request");

	

	if (!allCharactersAllowed(message.uri.fullUri, URI_ALLOWED_CHARS))
		throw HttpError(BadRequest, "Bad Request");

	if (message.uri.fullUri.size() > 2048)
		throw HttpError(RequestURIToLong, "Request-URI Too Long");

	//413 error

	if (serverConfig.hasLocation(message.uri.path))
	{
		if (!serverConfig.getLocation(message.uri.path).hasMethod(message.method))
			throw HttpError(MethodNotAllowed, "Method Not Allowed");
	}
	else
		throw HttpError(NotFound, "Not Found");

}


// returns 1 if location has a redirect, 0 otherwise
bool RequestParser::parseUri(const std::string& uriStr) {
	Uri &uri = message.uri;

	size_t queryPos = uriStr.find('?');
	uri.path = uriStr.substr(0, queryPos);
	if (queryPos != std::string::npos) {
		uri.query = uriStr.substr(queryPos + 1);
	}
	location = serverConfig.getLocation(message.uri.path);
	fullLocation = location.root + &(message.uri.path[1]);
	if (!location._return.empty())
		return 1;
	checkPath();	

	std::string fileExtention = getFileExtention(message.uri.path);
	if (location.cgi_path.find(fileExtention) != location.cgi_path.end()){
		CGIpath = location.cgi_path[fileExtention];
		isCGIfile = true;
	}
	return 0;
}

void RequestParser::checkPath()
{
	struct stat fileInfo;
	if (access(fullLocation.c_str(), F_OK) == 0){
		stat(fullLocation.c_str(), &fileInfo);
		if (S_ISDIR(fileInfo.st_mode))
			isDir = 1;
		else if (access(fullLocation.c_str(), R_OK) != 0)
			throw HttpError(Forbidden, "Forbidden");
	}
	else
		throw HttpError(NotFound, "Not Found");
}
