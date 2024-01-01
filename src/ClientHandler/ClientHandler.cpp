#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientFd, int epollFd ,const ServerConfig &serverConfig, const ClusterConfig &clusterConfig)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->headersLoaded = false;
	this->closed = false;
	this->serverConfig = serverConfig;
	this->clusterConfig = clusterConfig;
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
	std::string httpResponse = "HTTP/1.1 200 OK \r\n"
							   "Content-Type: text/plain\r\n"
							   "Content-Length: 12\r\n"
							   "\r\n"
							   "Hello, World";

	int bytesSent = ::send(this->clientFd, httpResponse.c_str(), httpResponse.size(), 0);
	if (bytesSent <= -1) {
		std::cerr << "Error sending data: " << strerror(errno) << "\n";
		closeConnection();
		return;
	}

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
