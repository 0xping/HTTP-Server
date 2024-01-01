#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientFd, int epollFd ,const ServerConfig &serverConfig, const ClusterConfig &clusterConfig)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->headersLoaded = false;
	this->closed = false;
	this->serverConfig = serverConfig;
	this->clusterConfig = clusterConfig;
	this->Offset = 0;
}

void ClientHandler::refresh()
{
	std::cout << "Server: Client Refreshed\n";
	if (!headersLoaded)
	{
		std::cout << "loading HTTP Headers \n\n\n";
		readFromSocket();
		if (closed)
			return ;
		// Check for end of HTTP headers (double newline or double \r\n)
		size_t headerEnd = std::min(toRead.find("\r\n\r\n"), toRead.find("\n\n"));
		if (headerEnd != std::string::npos)
		{
			// Headers received, process the headers
			std::string headersStr = toRead.substr(0, headerEnd);
			loadHeaders(headersStr);
			// Determine the start of the body
			size_t bodyStart = headerEnd == toRead.find("\r\n\r\n") ? headerEnd + 4 : headerEnd + 2;
			toRead = toRead.substr(bodyStart);
		}
	}
	else
	{
		if(message.headers.find("Host") != message.headers.end())
			serverConfig = clusterConfig.getServerConfig(serverConfig.ip, serverConfig.port, message.headers["host"]);
		//headers are parsed
		//check the method in <message.method>
		//and pass the call to the method, HAYTHAM ATACK LOO

		//fake response and then close the connection

		// std::string httpResponse = "HTTP/1.1 200 OK \r\n"
		// 						"Content-Type: text/plain\r\n"
		// 						"Content-Length: 12\r\n"
		// 						"\r\n" "Hello, World";
		// send(this->clientFd, httpResponse.c_str(), httpResponse.size(), 0);
		std::map<std::string, std::string> nn;
		nn["test"] = "niggah";
		SendResponse("200", nn, "index.html");
	}

	// print headers and buffered
	std::cout << "-------> Headers <------- \n\n";
	for (std::map<std::string, std::string>::const_iterator it = message.headers.begin(); it != message.headers.end(); ++it) {
		std::cout << it->first << ":" << it->second << '\n';
	}

	std::cout << "\n\n-------> Body  <-------\n" << toRead << '\n';
}

void ClientHandler::readFromSocket(int bufferSize)
{
	char buffer[bufferSize];
	std::memset(buffer, 0, bufferSize);
	ssize_t bytesRead = read(this->clientFd, buffer, bufferSize - 1);

	if (bytesRead <= 0)
	{
		if (bytesRead == 0) std::cout << "No bytes to Read, or Connection closed by client\n";
		if (bytesRead == -1) std::cerr << "Error receiving data: " << strerror(errno) << "\n";
		// closeConnection();
		return;
	}
	this->toRead.append(buffer);
}

int ClientHandler::loadHeaders(const std::string &data)
{
	//TODO :: trim the data first || NOT NEEDED
	std::vector<std::string> delimiters;
	delimiters.push_back("\n\n");
	delimiters.push_back("\r\n");

	std::vector<std::string> lines = splitWithDelimiters(data, delimiters);
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (i == 0)
		{
			//check the first line
			std::vector<std::string> words = strSplit(lines[i], " ");
			if (words.size() != 3)
				;// error bad request
			message.method = words[0];
			message.location = words[1];
			proccessLocation();
		}
		else
		{
			size_t colonPos = lines[i].find(':');
			std::string key = lines[i].substr(0, colonPos);
			std::string value = (colonPos != std::string::npos) ? lines[i].substr(colonPos + 1) : "";
			message.headers[key] = value;
		}
	}
	this->headersLoaded = true;
	return 0;
}

void ClientHandler::proccessLocation(){
	std::string* splitted = split(message.location, '?'); 

	location = serverConfig.getLocation(message.location);
	query = splitted[1];
	full_location = location.root + &(splitted[0][1]);	
	
	delete[] splitted;
}

void ClientHandler::closeConnection()
{
	epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
	close(clientFd);
	closed = true;
}


void ClientHandler::SendResponse(std::string statusCode, std::map<std::string, std::string> headers, std::string file, bool isCgi){
	std::string re;
	std::ifstream fileToSend;
	std::vector<std::string> errorPageNStatus = serverConfig.getErrorPage(statusCode);

	if (!file.empty())
		fileToSend.open(file.c_str());
	else{
		file = errorPageNStatus[0];
		fileToSend.open(file.c_str());
	}
	fileToSend.seekg(Offset);		
	char buffer[1024];
	if (!headersSent){
		re = generateHeaders(statusCode, headers, file, fileToSend, isCgi);

		if (re.length() <= 1024){
			fileToSend.read(buffer, 1024 - re.length());
			Offset = fileToSend.tellg();
			re += buffer;
		}
		else{
			toSend = &re[1024];
			re.resize(1024);
		}
		send(clientFd, re.c_str(), re.length(), 0);
		headersSent = 1;
	}
	else{
		if (toSend.length() > 1024){
			send(clientFd, toSend.c_str(), 1024, 0);
			toSend = &toSend[1024];
		}
		else{
			fileToSend.read(buffer, 1024 - toSend.length());
			Offset = fileToSend.tellg();
			toSend += buffer;
			send(clientFd, toSend.c_str(), toSend.length(), 0);
			toSend.clear();
		}
	}
	if (fileToSend.eof() && toSend.empty()){
		if (isCgi)
			std::remove(file.c_str());
		closeConnection();
	}
	
	fileToSend.close();
	exit(1);
}

std::string ClientHandler::generateHeaders(std::string& statusCode, std::map<std::string, std::string>& headers, std::string& filename, std::ifstream& file, bool isCgi){
	std::string re;
	std::vector<std::string> errorPageNStatus = serverConfig.getErrorPage(statusCode);

	re = "http/1.1 " + statusCode + " " + errorPageNStatus[1] + "\r\n";
	for (std::map<std::string,std::string>::iterator it = headers.begin(); it != headers.end(); it++){
			re += it->first + ": " + it->second + "\r\n";
	}
	re += "Content-Type: " + getMimeType(getExtension(filename)) + "\r\n";		
	re += "Content-Length: " + getContentLength(file) + "\r\n";
	if (!isCgi)
		re += "\r\n";
	return re;
}

std::string ClientHandler::getExtension(std::string& filename){
	std::vector<std::string> splitted = strSplit(filename, ".");
	return splitted[splitted.size() - 1];
}

std::string ClientHandler::getContentLength(std::ifstream& file){	
	std::stringstream ss;
	std::streampos originalPos = file.tellg();

	file.seekg(0, std::ios_base::end);
	ss << file.tellg();
	file.seekg(originalPos);
	return ss.str();
}

std::string ClientHandler::getMimeType(std::string ext){
	std::map<std::string, std::string> mimeTypes;
    
    mimeTypes["txt"] = "text/plain";
    mimeTypes["html"] = "text/html";
    mimeTypes["css"] = "text/css";
    mimeTypes["js"] = "application/javascript";
    mimeTypes["jpg"] = "image/jpeg";
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

bool ClientHandler::isCgiFile(std::string& filename){
	std::string ext = getExtension(filename);
	if (location.cgi_path.find(ext) != location.cgi_path.end()){
		cgi_path = location.cgi_path[ext]; 
		return 1;
	}
	return 0;
}

void ClientHandler::execCGI(std::string& filepath){
	std::srand(std::time(0)); 
	std::stringstream ss;
	ss << std::rand();
	std::string cgioutput = "/tmp/cgioutput" + ss.str() + ".txt";
	pid_t pid = fork(); 
	if (pid){
		if (pid < 0)
			SendResponse("500", std::map<std::string, std::string>(), "");
		int status;
		clock_t startTime = clock();
		while (waitpid(pid, &status, WNOHANG) == 0){
			clock_t endTime = clock();
			double elapsedTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;
			if (elapsedTime >= 30){
				SendResponse("500", std::map<std::string, std::string>(), "");
				kill(pid, SIGTERM);
				return ;
			}
		}
    	if (WIFEXITED(status)){
			if (!WEXITSTATUS(status))
				SendResponse("200", std::map<std::string, std::string>(), cgioutput);
			else
				SendResponse("500", std::map<std::string, std::string>(), "");
		}
		else if (WIFSIGNALED(status))
			SendResponse("500", std::map<std::string, std::string>(), "");
    	
	}
	else{
		int fd = open(filepath.c_str(), O_WRONLY);
		if (fd < 0)
			exit(1);
		const char *args[] = {cgi_path.c_str(), filepath.c_str(), NULL};
		const char *env[] = {query.c_str(), NULL};
		if (dup2(fd,1) == -1)
			exit(1);
		execve(filepath.c_str(), (char *const *)args, (char *const *)env);
		exit(1);
	}
}