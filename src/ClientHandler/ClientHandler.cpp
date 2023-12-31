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
		if(message.headers.find("host") != message.headers.end())
			serverConfig = clusterConfig.getServerConfig(serverConfig.ip, serverConfig.port, message.headers["host"]);
		//headers are parsed
		//check the method in <message.method>
		//and pass the call to the method, HAYTHAM ATACK LOO

		//fake response and then close the connection

		std::string httpResponse = "HTTP/1.1 200 OK \r\n"
								"Content-Type: text/plain\r\n"
								"Content-Length: 12\r\n"
								"\r\n" "Hello, World";
		send(this->clientFd, httpResponse.c_str(), httpResponse.size(), 0);
		closeConnection();
	}

	// print headers and buffered
	std::cout << "-------> Headers <------- \n\n";
	for (std::map<std::string, std::string>::const_iterator it = message.headers.begin(); it != message.headers.end(); ++it) {
		std::cout << it->first << ":" << it->second << '\n';
	}

	std::cout << "\n\n-------> Body  <-------\n" << toRead << '\n';
}
#include <unistd.h>

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

void ClientHandler::closeConnection()
{
	epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
	close(clientFd);
	closed = true;
}