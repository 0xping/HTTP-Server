#include "RequestParser.hpp"

RequestParser::RequestParser()
{
	this->headersLoaded = false;
	this->isCGIfile = false;
	this->contentLength = 0;
	
	
}


int RequestParser::loadHeaders(Binary &data)
{
	while(data.toStr().length() && data.toStr()[0] == '\n')
		data.erase(0,1);

	size_t headerEnd = std::min(data.toStr().find("\r\n\r\n"), data.toStr().find("\n\n"));
	if (headerEnd != std::string::npos) {
		std::string headersStr = data.toStr().substr(0, headerEnd);
		std::vector<std::string> delimiters;
		delimiters.push_back("\n");
		delimiters.push_back("\r\n");

		std::vector<std::string> lines = splitWithDelimiters(headersStr, delimiters);
		for (size_t i = 0; i < lines.size(); i++)
		{
			if (i == 0)
				checkRequestLine(lines[0]);
			else
				checkHeader(lines[i]);
		}
		this->headersLoaded = true;
		size_t bodyStart = headerEnd == data.find("\r\n\r\n") ? headerEnd + 4 : headerEnd + 2;
		data = data.substr(bodyStart);
	}
	return 0;
}

void RequestParser::checkHeader(std::string &header)
{
	size_t colonPos = header.find(':');
	std::string key = header.substr(0, colonPos);
	std::string value = (colonPos != std::string::npos) ? header.substr(colonPos + 1) : "";
	if (key.find(' ') != std::string::npos)
		throw HttpError(BadRequest, "Bad Request");
	if (key == "Host" && message.headers.find("Host") != message.headers.end())
		throw HttpError(BadRequest, "Bad Request");
	message.headers[strtrim(key)] = strtrim(value);
}


void RequestParser::checkRequestLine(std::string& requestLine)
{
	std::vector<std::string> words = strSplit(requestLine, " ",0);

	if (requestLine[0] != ' ' && words.size() == 3)
	{
		requestLine = requestLine.substr(requestLine.find_first_not_of(" \t"));
		std::istringstream requestLineStream(requestLine);
		std::string httpVersion;
		requestLineStream >> message.method >> message.uri.unparsedURI >> httpVersion;
		message.uri.fullUri = urlDecode(message.uri.unparsedURI);
		words = strSplit(httpVersion, "/", 0);
		if (words.size() != 2 || words[0] != "HTTP")
			throw HttpError(BadRequest, "Bad Request check the line request");
		if (words[1] != "1.1")
			throw HttpError(HTTPVersionNotSupported ,"505 HTTP Version Not Supported");
	}
	else
		throw HttpError(BadRequest, "Bad Request request line 1");
}


void RequestParser::parseRequest()
{
	std::map<std::string, std::string>::iterator TransferEncoding = message.headers.find("Transfer-Encoding");
	std::map<std::string, std::string>::iterator contentLength = message.headers.find("Content-Length");

	if (message.method != "GET" && message.method != "POST" && message.method != "DELETE")
		throw HttpError(NotImplemented, "Not Implemented");

	if (parseUri(message.uri.fullUri)) // returns true if location has a redirect
		return ;
	if (serverConfig.hasLocation(message.uri.path))
	{
		if (!serverConfig.getLocation(message.uri.path).hasMethod(message.method))
			throw HttpError(MethodNotAllowed, "Method Not Allowed");
	}
	else
		throw HttpError(NotFound, "Not Found");
	if (message.method == "POST")
	{
		if (TransferEncoding != message.headers.end())
		{
			if (TransferEncoding->second != "chunked")
				throw HttpError(NotImplemented, "Not Implemented");
		}
		else if (contentLength == message.headers.end())
			throw HttpError(BadRequest, "Bad Request no contentLength header exist");
		else if (!isValidBase(contentLength->second, this->contentLength, 10))
			throw HttpError(BadRequest, "Bad Request");
	}
	else
	{
		if (!isValidBase(contentLength->second, this->contentLength, 10))
			throw HttpError(BadRequest, "Bad Request");
		else if (this->contentLength > this->serverConfig.max_body_size)
			throw HttpError(PayloadTooLarge, "Payload Too Large");
	}
	if (!allCharactersAllowed(message.uri.unparsedURI, URI_ALLOWED_CHARS))
		throw HttpError(BadRequest, "Bad Request uri contains none allowed chars " + message.uri.unparsedURI);

	if (message.uri.unparsedURI.size() > 2048)
		throw HttpError(RequestURIToLong, "Request-URI Too Long");
		
	
	//413 error
}


// returns 1 if location has a redirect, 0 otherwise
bool RequestParser::parseUri(const std::string& uriStr) {
	Uri &uri = message.uri;

	size_t queryPos = uriStr.find('?');
	uri.path = uriStr.substr(0, queryPos);
	if (queryPos != std::string::npos) {
		query = uriStr.substr(queryPos + 1);
	}
	location = serverConfig.getLocation(message.uri.path);
	if (this->location.upload_path.empty())
		this->upload_path = "./root/UPLOADS";
	else
		this->upload_path = this->location.upload_path;
	fullLocation = location.root + message.uri.path;
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
		if (access(fullLocation.c_str(), R_OK) != 0)
			throw HttpError(Forbidden, "Forbidden");
	}
	else
		throw HttpError(NotFound, "Not Found");
}
