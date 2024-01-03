#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientFd, int epollFd ,const ServerConfig &serverConfig, const ClusterConfig &clusterConfig)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->headersLoaded = false;
	this->closed = false;
	this->serverConfig = serverConfig;
	this->clusterConfig = clusterConfig;
	//total = 0;
}

void ClientHandler::receive() {
	// Client ready to receive data
	std::cout << "ClientHandler: Client Receiving...\n";
	if (!headersLoaded) {
		readFromSocket();
		if (closed)
			return;
		if (loadHeaders())
			return; // Bad request
	}

	std::cout << "Headers Loaded" << std::endl;
}

void ClientHandler::send() {
	// Client ready to send data
	if (!headersLoaded) {
		return;
	}

	std::cout << "ClientHandler: Client Sending...\n";

	// Send the response
	// std::string httpResponse = "HTTP/1.1 200 OK \r\n"
	// 						   "Content-Type: text/plain\r\n"
	// 						   "Content-Length: 12\r\n"
	// 						   "\r\n"
	// 						   "Hello, World";
	while (!closed){
		std::cout << full_location << std::endl;
		SendResponse("200", std::map<std::string, std::string>(), full_location);
	}
	std::cout << "----> out of loop! closed == " << closed << std::endl;
	// int bytesSent = ::send(this->clientFd, httpResponse.c_str(), httpResponse.size(), 0);
	// if (bytesSent <= -1) {
	// 	std::cerr << "Error sending data: " << strerror(errno) << "\n";
	// 	closeConnection();
	// 	return;
	// }

}

void ClientHandler::readFromSocket(int bufferSize) {
	unsigned char buffer[bufferSize];
	std::memset(buffer, 0, bufferSize);
	ssize_t bytesRead = recv(this->clientFd, buffer, bufferSize - 1, 0);

	if (bytesRead <= 0) {
		if (bytesRead == 0)
			std::cout << "No bytes to Read, or Connection closed by client\n";
		if (bytesRead == -1)
			std::cerr << "Error receiving data: " << strerror(errno) << "\n";
		closeConnection();
		return;
	}
	this->toRead.append(buffer, bytesRead);
}

void ClientHandler::closeConnection() {
	closed = true;
	// epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
	// close(clientFd);
}

int ClientHandler::loadHeaders() {
	if (!headersLoaded) {
		std::cout << "Loading HTTP Headers \n\n\n";
		// Check for the end of HTTP headers (double newline or double \r\n)
		size_t headerEnd = std::min(toRead.toStr().find("\r\n\r\n"), toRead.toStr().find("\n\n"));
		if (headerEnd != std::string::npos) {
			// Headers received, process the headers
			std::string headersStr = toRead.toStr().substr(0, headerEnd);
			// TODO: Trim the data first || NOT NEEDED
			std::vector<std::string> delimiters;
			delimiters.push_back("\n\n");
			delimiters.push_back("\r\n");

			std::vector<std::string> lines = splitWithDelimiters(headersStr, delimiters);
			for (size_t i = 0; i < lines.size(); i++) {
				if (i == 0) {
					// Check the first line
					std::vector<std::string> words = strSplit(lines[i], " ");
					// if (words.size() != 3)
					//	 ; // Error bad request
					message.method = words[0];
					message.location = words[1];
					if(message.headers.find("Host") != message.headers.end())
						serverConfig = clusterConfig.getServerConfig(serverConfig.ip, serverConfig.port, message.headers["host"]);
					proccessLocation();
					checkPath();
					
				} else {
					size_t colonPos = lines[i].find(':');
					std::string key = lines[i].substr(0, colonPos);
					std::string value = (colonPos != std::string::npos) ? lines[i].substr(colonPos + 1) : "";
					message.headers[key] = value;
				}
			}
			this->headersLoaded = true;
			// Determine the start of the body
			size_t bodyStart = headerEnd == toRead.find("\r\n\r\n") ? headerEnd + 4 : headerEnd + 2;
			toRead = toRead.substr(bodyStart);
		}
	}
	return 0;
}

// std::cout << "-------> Headers <------- " << message.headers.size() << "\n\n";
// for (std::map<std::string, std::string>::const_iterator it = message.headers.begin(); it != message.headers.end(); ++it) {
//	 std::cout << it->first << ":" << it->second << '\n';
// }

// std::cout << "\n\n-------> Body  <-------\n" << toRead << '\n';

void ClientHandler::proccessLocation(){
	std::string* splitted = split(message.location, '?');
	std::string delimeter = "/";
	std::vector<std::string> strVec = strSplit(splitted[0], delimeter);

	location = serverConfig.getLocation(message.location);
	query = splitted[1];
	full_location = location.root + &(splitted[0][1]);	
	isCgipath = isCgiFile();
	
	delete[] splitted;
}

void ClientHandler::SendResponse(std::string statusCode, std::map<std::string, std::string> headers, std::string file, bool isCgi){
	char buffer[BUFFER_SIZE + 1] =  {0};
	int readLen = 0;

	if (!headersSent){
		isCgiSending = isCgi;	
		std::string re;
		std::vector<std::string> errorPageNStatus = serverConfig.getErrorPage(statusCode);
		if (!file.empty()){
			FileName = file;
			toSendFd = open(file.c_str(), O_RDONLY);
		}
		else{
			FileName = errorPageNStatus[0];
			toSendFd = open(errorPageNStatus[0].c_str(), O_RDONLY);
		}

		re = generateHeaders(statusCode, headers, isCgi);

		readLen = read(toSendFd, buffer, BUFFER_SIZE - re.length());
		re += buffer;

		::send(clientFd, re.c_str(), re.length(), 0);
		headersSent = 1;
	}
	else{
		readLen = read(toSendFd, buffer, BUFFER_SIZE); 	
		::send(clientFd, buffer, readLen, 0);
	}
	// std::cout << "-> read len: " << readLen << std::endl;
	//total += readLen;
	if (!readLen){
		//std::cout << "here" << std::endl;
		if (isCgiSending)
			std::remove(FileName.c_str());
		closeConnection();
		close(toSendFd);
		//std::cout << "total = " << total << std::endl;
		//exit(1);
	}
}

std::string ClientHandler::generateHeaders(std::string& statusCode, std::map<std::string, std::string>& headers, bool isCgi){
	std::string re;
	std::vector<std::string> errorPageNStatus = serverConfig.getErrorPage(statusCode);

	re = "http/1.1 " + statusCode + " " + errorPageNStatus[1] + "\r\n";
	for (std::map<std::string,std::string>::iterator it = headers.begin(); it != headers.end(); it++){
			re += it->first + ": " + it->second + "\r\n";
	}
	re += "Content-Type: " + getMimeType(getExtension()) + "\r\n";		
	re += "Content-Length: " + getContentLength() + "\r\n";
	if (!isCgi)
		re += "\r\n";
	return re;
}

std::string ClientHandler::getExtension(){
	if (FileName.find(".") == std::string::npos)
		return "";
	std::vector<std::string> splitted = strSplit(FileName, ".");
	return splitted[splitted.size() - 1];
}

std::string ClientHandler::getContentLength(){
	std::stringstream ss;	
    struct stat file_info;
    fstat(toSendFd, &file_info);
	ss << file_info.st_size;
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

bool ClientHandler::isCgiFile(){
	std::string ext = getExtension();
	if (location.cgi_path.find(ext) != location.cgi_path.end()){
		cgi_path = location.cgi_path[ext]; 
		return 1;
	}
	return 0;
}

void ClientHandler::execCGI(){
	std::srand(std::time(0)); 
	std::stringstream ss;
	ss << std::rand();
	std::string cgioutput = "/tmp/cgioutput" + ss.str() + ".txt";
	pid_t pid = fork(); 
	if (pid){
		if (pid < 0){
			std::remove(cgioutput.c_str());
			SendResponse("500", std::map<std::string, std::string>(), "");
		}
		int status;
		clock_t startTime = clock();
		while (waitpid(pid, &status, WNOHANG) == 0){
			clock_t endTime = clock();
			double elapsedTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
			if (elapsedTime >= 30){
				std::remove(cgioutput.c_str());
				SendResponse("500", std::map<std::string, std::string>(), "");
				kill(pid, SIGTERM);
				return ;
			}
		}
		
    	if (WIFEXITED(status)){
			if (!WEXITSTATUS(status))
				SendResponse("200", std::map<std::string, std::string>(), cgioutput, 1);
			else{
				std::remove(cgioutput.c_str());
				SendResponse("500", std::map<std::string, std::string>(), "");
			}
		}
		else if (WIFSIGNALED(status)){
			std::remove(cgioutput.c_str());
			SendResponse("500", std::map<std::string, std::string>(), "");
		}
    	
	}
	else{
		int fd = open(full_location.c_str(), O_WRONLY);
		if (fd < 0)
			exit(1);
		const char *args[] = {cgi_path.c_str(), full_location.c_str(), NULL};
		const char *env[] = {query.c_str(), NULL};
		if (dup2(fd,1) == -1)
			exit(1);
		execve(full_location.c_str(), (char *const *)args, (char *const *)env);
		exit(1);
	}
}


void ClientHandler::checkPath(){
	struct stat fileInfo;

	if (access(full_location.c_str(), F_OK) == 0){
		stat(full_location.c_str(), &fileInfo);
		if (S_ISDIR(fileInfo.st_mode))
			isDir = 1;
		else if (access(full_location.c_str(), R_OK) != 0){
			SendResponse("403", std::map<std::string, std::string>(), "");
			std::cout << "+++++++++++++++++++++ hna \n"<<std::endl;
		}
	}
	else{
		std::cout << "+++++++++++++++++++++error\n"<<std::endl;
		SendResponse("404", std::map<std::string, std::string>(), "index.html");
	}
}