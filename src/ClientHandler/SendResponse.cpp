#include "ClientHandler.hpp"

// response
void ClientHandler::SendResponse(){
	if (!headersSent){
		if (file.empty())
			file = serverConfig.getErrorPage(statusCode);
		if (access(file.c_str(), R_OK)){
			if (statusCode == "500"){
				sendServerError();
				return ;
			}
			throw HttpError(InternalServerError, "Internal Server Error");
		}
		std::string re;

		re = generateHeaders();

		sendingBuffer.append((const unsigned char *)re.c_str(),std::strlen(re.c_str()));
		sendToSocket();

		headersSent = 1;
	}
	else{
		//protect
		// std::cout << "response: " << file.c_str() << std::endl;
		std::ifstream fileToSend(file.c_str(), std::ios::binary);
		if (!fileToSend.is_open()){
			status = Closed;
			return ;
		}
			
		char buffer[BUFFER_SIZE + 1] =  {0};

		fileToSend.seekg(offset);
		fileToSend.read(buffer, BUFFER_SIZE);

		offset = fileToSend.tellg();
		int readLen = fileToSend.gcount();

		sendingBuffer.append((unsigned const char*)buffer, readLen);
		sendToSocket();

		/// set status to Closed if done sending
		if (fileToSend.eof())
			status = Closed;
		fileToSend.close();
	}
}

std::string ClientHandler::generateHeaders(){
	std::string re;

	re = "HTTP/1.1 " + statusCode + " " + statusString + "\r\n";
	re += "Content-Type: " + getMimeType(getExtension()) + "\r\n";
	if (!isCGI)
		re += "Content-Length: " + getContentLength() + "\r\n";
	re += extraHeaders;
	if (!isCGI)
		re += "\r\n";
	return re;
}

std::string ClientHandler::getExtension(){
	if (file.find(".") == std::string::npos)
		return "";
	std::vector<std::string> splitted = strSplit(file, ".", 1);
	return splitted[splitted.size() - 1];
}

std::string ClientHandler::getContentLength(){
	struct stat fileInfo;

	stat(file.c_str(), &fileInfo);

	std::stringstream ss;
	ss << fileInfo.st_size;

	return ss.str();
}

std::string ClientHandler::getMimeType(std::string ext){
	std::map<std::string, std::string> mimeTypes;

	mimeTypes["txt"] = "text/plain";
	mimeTypes["html"] = "text/html";
	mimeTypes["css"] = "text/css";
	mimeTypes["js"] = "application/javascript";
	mimeTypes["jpg"] = "image/jpeg";
	mimeTypes["webp"] = "image/webp";
	mimeTypes["png"] = "image/png";
	mimeTypes["gif"] = "image/gif";
	mimeTypes["pdf"] = "application/pdf";
	mimeTypes["doc"] = "application/msword";
	mimeTypes["xml"] = "application/xml";
	mimeTypes["json"] = "application/json";
	mimeTypes["mp3"] = "audio/mpeg";
	mimeTypes["csv"] = "text/csv";
	mimeTypes["zip"] = "application/zip";
	mimeTypes["tar"] = "application/x-tar";
	mimeTypes["mp4"] = "video/mp4";
	mimeTypes["ogg"] = "audio/ogg";
	mimeTypes["svg"] = "image/svg+xml";
	mimeTypes["xls"] = "application/vnd.ms-excel";
	mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
	mimeTypes["ico"] = "image/x-icon";
	mimeTypes["woff"] = "font/woff";
	mimeTypes["mpg"] = "video/mpeg";

	std::map<std::string,std::string>::iterator it = mimeTypes.find(ext);
	if (it != mimeTypes.end())
		return it->second;
	return "text/plain";
}

std::string ClientHandler::getExtensionPost(std::string mimeType){
	std::map<std::string, std::string> mimeTypes;

	mimeTypes["text/plain"] = ".txt";
	mimeTypes["text/html"] = ".html";
	mimeTypes["text/css"] = ".css";
	mimeTypes["application/javascript"] = ".js";
	mimeTypes["image/jpeg"] = ".jpg";
	mimeTypes["image/webp"] = ".webp";
	mimeTypes["image/png"] = ".png";
	mimeTypes["image/gif"] = ".gif";
	mimeTypes["application/pdf"] = ".pdf";
	mimeTypes["application/msword"] = ".doc";
	mimeTypes["application/xml"] = ".xml";
	mimeTypes["application/json"] = ".json";
	mimeTypes["audio/mpeg"] = ".mp3";
	mimeTypes["text/csv"] = ".csv";
	mimeTypes["application/zip"] = ".zip";
	mimeTypes["application/x-tar"] = ".tar";
	mimeTypes["video/mp4"] = ".mp4";
	mimeTypes["audio/ogg"] = ".ogg";
	mimeTypes["image/svg+xml"] = ".svg";
	mimeTypes["application/vnd.ms-excel"] = ".xls";
	mimeTypes["application/vnd.ms-powerpoint"] = ".ppt";
	mimeTypes["image/x-icon"] = ".ico";
	mimeTypes["font/woff"] = ".woff";
	mimeTypes["video/mpeg"] = ".mpg";



	std::map<std::string,std::string>::iterator it = mimeTypes.find(mimeType);
	if (it != mimeTypes.end())
		return it->second;
	return ".tmp";
}

void ClientHandler::setResponseParams(std::string statusCode, std::string statusString, std::string extraHeaders, std::string file, bool isCGI){
	this->file = file;
	this->statusCode = statusCode;
	this->extraHeaders += extraHeaders;
	this->statusString = statusString;
	this->isCGI = isCGI;
	status = Sending;
}


void ClientHandler::sendServerError(){ 
    // Content to be sent in the HTTP response body
    const char* response =
		"HTTP/1.1 500 Internal Server Error\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 1158\r\n\r\n"
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "\t<meta charset=\"UTF-8\">\n"
        "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "\t<title>500 Internal Server Error</title>\n"
        "\t<style>\n"
        "\t\tbody {\n"
        "\t\t  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;\n"
        "\t\t  background-color: #222222; /* Deeper, more elegant dark background */\n"
        "\t\t  color: #f0f0f0; /* Slightly brighter text for better readability */\n"
        "\t\t  text-align: center;\n"
        "\t\t  padding: 50px;\n"
        "\t\t  margin: 0;\n"
        "\t\t}\n"
        "\n"
        "\t\th1 {\n"
        "\t\t  color: #e74c3c;\n"
        "\t\t  text-shadow: 2px 2px 4px #333;\n"
        "\t\t  font-size: 3em; /* Larger heading for greater impact */\n"
        "\t\t}\n"
        "\n"
        "\t\tp {\n"
        "\t\t  font-size: 1.2em; /* Slightly larger text for easier reading */\n"
        "\t\t  color: #a8a8a8;\n"
        "\t\t  margin-bottom: 20px;\n"
        "\t\t  text-shadow: 1px 1px 2px #333;\n"
        "\t\t}\n"
        "\n"
        "\t\ta {\n"
        "\t\t  color: #3498db;\n"
        "\t\t  text-decoration: none;\n"
        "\t\t  transition: color 0.3s;\n"
        "\t\t}\n"
        "\n"
        "\t\ta:hover {\n"
        "\t\t  color: #2980b9;\n"
        "\t\t  text-decoration: underline;\n"
        "\t\t}\n"
        "\t  </style>\n"
        "</head>\n"
        "<body>\n"
        "\t<h1>500 - Internal Server Error</h1>\n"
        "\t<p>Oops! Something went wrong on our server.</p>\n"
        "\t<p>Please try again later or contact <a href=\"https://profile.intra.42.fr/users/aait-lfd\" style=\"color: #3498db;\">support</a>.</p>\n"
        "</body>\n"
        "</html>";

	sendingBuffer.append((const unsigned char *)response,std::strlen(response));
	sendToSocket();
	status = Closed;
}