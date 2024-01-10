#include "ClientHandler.hpp"

// response
void ClientHandler::SendResponse(){
	if (!headersSent){
		if (file.empty()){
            std::string errorPage = serverConfig.getErrorPage(statusCode);
			file = errorPage;
		}
		std::string re;

		re = generateHeaders();

		sendingBuffer.append((const unsigned char *)re.c_str(),std::strlen(re.c_str()));
		sendToSocket();

		headersSent = 1;
	}
	else{		
		//protect	
		std::ifstream fileToSend(file.c_str(), std::ios::binary);        
		char buffer[BUFFER_SIZE + 1] =  {0};        

		fileToSend.seekg(offset);
		fileToSend.read(buffer, BUFFER_SIZE);
		offset = fileToSend.tellg();
		// check g count, protect
		int readLen = fileToSend.gcount();	
	
		sendingBuffer.append((unsigned const char*)buffer, readLen);
		sendToSocket();

		/// set status to Closed if done sending
		if (!readLen)
		{
			//sleep(2);
			headersSent = 0;
			status = Closed;
		}

		fileToSend.close();
	}
}

std::string ClientHandler::generateHeaders(){
	std::string re;

	re = "HTTP/1.1 " + statusCode + " " + statusString + "\r\n";
	re += "Content-Type: " + getMimeType(getExtension()) + "\r\n";		
	re += "Content-Length: " + getContentLength() + "\r\n";
	re += extraHeaders;
	//if (!isCGI)
	re += "\r\n";
	return re;
}

std::string ClientHandler::getExtension(){
	if (file.find(".") == std::string::npos)
		return "";
	std::vector<std::string> splitted = strSplit(file, ".");
	return splitted[splitted.size() - 1];
}

std::string ClientHandler::getContentLength(){
	std::stringstream ss;	
	std::ifstream tmpifstream(file.c_str(), std::ios::binary);
	tmpifstream.seekg(0, std::ios::end);
	ss << tmpifstream.tellg();
	tmpifstream.close();
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

void ClientHandler::setResponseParams(std::string statusCode, std::string statusString, std::string extraHeaders, std::string file, bool isCGI){
    this->file = file;
    this->statusCode = statusCode;
    this->extraHeaders = extraHeaders;
	this->statusString = statusString;
    this->isCGI = isCGI;
	status = Sending;
}
