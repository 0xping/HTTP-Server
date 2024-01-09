#include "RequestParser.hpp"

RequestParser::RequestParser()
{
	this->headersLoaded = false;
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

	if (TransferEncoding != message.headers.end())
	{
		if (TransferEncoding->second != "chunked")
			throw HttpError(NotImplemented, "Not Implemented");
	}
	else if (contentLength == message.headers.end() && message.method == "POST")
		throw HttpError(BadRequest, "Bad Request");

	parseUri(message.uri.fullUri);

	if (!allCharactersAllowed(message.uri.fullUri, URI_ALLOWED_CHARS))
		throw HttpError(BadRequest, "Bad Request");

	if (message.uri.fullUri.size() > 2048)
		throw HttpError(RequestURIToLong, "Request-URI To Long");

	//413 error

	if (serverConfig.hasLocation(message.uri.path))
	{
		if (!serverConfig.getLocation(message.uri.path).hasMethod(message.method))
			throw HttpError(MethodNotAllowed, "Method Not Allowed");
	}
	else
		throw HttpError(NotFound, "Not Found");

	// proccessLocation();
	// checkPath();
}




void RequestParser::parseUri(const std::string& uriStr) {
	Uri &uri = message.uri;

	size_t queryPos = uriStr.find('?');
	uri.path = uriStr.substr(0, queryPos);
	if (queryPos != std::string::npos) {
		uri.query = uriStr.substr(queryPos + 1);
	}

	location = serverConfig.getLocation(message.uri.path);
	fullLocation = location.root + message.uri.path;
	std::string fileExtention = getFileExtention("dsa");
	if (location.cgi_path.find(fileExtention) != location.cgi_path.end()){
		CGIpath = location.cgi_path[fileExtention];
	}
}


// void RequestParser::proccessLocation()
// {

// 	std::string* splitted = split(message.uri, '?');
// 	std::vector<std::string> strVec = strSplit(splitted[0], "/");

// 	location = serverConfig.getLocation(message.uri);
// 	query = splitted[1];
// 	full_location = location.root + &(splitted[0][1]);
// 	isCGIpath = isCGIFile();

// 	delete[] splitted;
// }


// void RequestParser::checkPath()
// {
// 	struct stat fileInfo;

// 	std::cout << full_location << std::endl;
// 	if (access(full_location.c_str(), F_OK) == 0){
// 		stat(full_location.c_str(), &fileInfo);
// 		if (S_ISDIR(fileInfo.st_mode))
// 			isDir = 1;
// 		else if (access(full_location.c_str(), R_OK) != 0)
// 			SendResponse("403", std::map<std::string, std::string>(), "");
// 	}
// 	else
// 		SendResponse("404", std::map<std::string, std::string>(), "");
// }

// bool RequestParser::isCGIFile(){
// 	std::string ext = getFileExtention();
// 	if (location.cgi_path.find(ext) != location.cgi_path.end()){
// 		cgi_path = location.cgi_path[ext];
// 		return 1;
// 	}
// 	return 0;
// }
