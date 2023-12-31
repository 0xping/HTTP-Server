#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientFd, int epollFd)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->headersLoaded = false;
	this->done = false;

}

void ClientHandler::refresh()
{
	std::cout << "Server: Client Refreshed\n";
	if (!headersLoaded)
	{
		std::cout << "loading HTTP Headers \n\n\n";
		readFromSocket();
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
		//headers are parsed
		//check the method in <message.method>
		//and pass the call to the method, HAYTHAM ATACK LOO

		//fake response and then close the connection
		const char *httpResponse = "HTTP/1.1 200 OK \r\n"
								   "Content-Type: text/plain\r\n"
								   "Content-Length: 12\r\n"
								   "\r\n"
								   "Hello, World";

		send(this->clientFd, httpResponse, strlen(httpResponse), 0);
		this->done = true;
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
	ssize_t bytesRead = recv(this->clientFd, buffer, bufferSize - 1, 0);

	if (bytesRead == 0)
	{
		std::cout << "Connection closed by client\n";
		this->done = true;
	}
	if (bytesRead == -1)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Error receiving data: " << strerror(errno) << "\n";
	}
	this->toRead.append(buffer);
}

void ClientHandler::closeSocket() {}

int ClientHandler::loadHeaders(const std::string &data)
{
	//TODO :: trim the data first
	std::vector<std::string> delimiters;
	delimiters.push_back("\n\n");
	delimiters.push_back("\r\n");

	std::vector<std::string> lines = splitWithDelimiters(data, delimiters);
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (i == 0)
		{
			//check the first line
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
	return 0; // Consider replacing this with a named constant
}