#include "ClientHandler.hpp"


ClientHandler::ClientHandler(int clientFd, int epollFd ,const ServerConfig &serverConfig, const ClusterConfig &clusterConfig)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->serverConfig = serverConfig;
	this->clusterConfig = clusterConfig;
	this->status = Receiving;
	this->postedFileName = "";


	this->headersSent = 0;
	this->offset = 0;
}


void ClientHandler::readyToReceive() {
	// Client ready to receive data
	try
	{
		if (!headersLoaded)
		{
			readFromSocket();
			loadHeaders(readingBuffer);
		}
		if (headersLoaded)
		{
			std::cout << "Headers Loaded" << std::endl;
			if (message.headers.find("Host") != message.headers.end()){
				serverConfig = clusterConfig.getServerConfig(serverConfig.ip, serverConfig.port, message.headers["Host"]);;
				RequestParser::serverConfig = serverConfig;
			}
			else
				throw HttpError(BadRequest, "Bad Request");
			parseRequest();
			if (message.method == "GET")
				GetMethod();
		}
		// check and call the method DELETE or POST <No GET>
		// to send a request form a method , just append to sendingBuffer
		//status = Sending;
	}
	catch (const HttpError& e)
	{
		// status = Sending; // status = Error;
		headersSent = 0;
		std::cerr << "HTTP Error (" << e.getErrorCode() << "): " << e.what() << std::endl;
		setResponseParams(toString(e.getErrorCode()), e.what(), "", "", false);
		// TODO : store the error state in the object and wait for a send event to come
	}
}

void ClientHandler::readyToSend() {
	// Client ready to send data
	try
	{
		SendResponse();
	}
	catch (const HttpError& e)
	{
		// status = Sending; // status = Error;
		headersSent = 0;
		int errorCode = static_cast<int>(e.getErrorCode());
		std::cerr << "HTTP Error (" << errorCode << "): " << e.what() << std::endl;
		setResponseParams(toString(e.getErrorCode()), e.what(), "", "", false); // check if send failed
		// TODO : store the error state in the object and wait for a send event to come
	}
}

void ClientHandler::readFromSocket(int bufferSize) {
	unsigned char buffer[bufferSize];
	std::memset(buffer, 0, bufferSize);
	ssize_t bytesRead = recv(this->clientFd, buffer, bufferSize - 1, 0);

	if (bytesRead <= 0) {
		if (bytesRead == 0)
		{
			std::cout << "Connection closed by client\n";
			status = Closed;
		}
		if (bytesRead == -1)
		{
			status = Error;
			std::cerr << "Error receiving data: " << strerror(errno) << "\n";
			throw HttpError(InternalServerError, "Internal Server Error");
		}
		return ;
	}
	this->readingBuffer.append(buffer, bytesRead);
}


void ClientHandler::sendToSocket()
{
	size_t totalBytesSent = 0;
	while (totalBytesSent < sendingBuffer.toStr().size())
	{
		ssize_t v = ::write(open("log", O_RDWR | O_CREAT | O_APPEND, 0777), sendingBuffer.toStr().c_str() + totalBytesSent, sendingBuffer.toStr().size() - totalBytesSent);
		v++;
		ssize_t sendBytes = ::write(this->clientFd, sendingBuffer.toStr().c_str() + totalBytesSent, sendingBuffer.toStr().size() - totalBytesSent);
		if (sendBytes <= 0)
		{
			if (sendBytes == 0)
			{
				std::cout << "Connection closed by client\n";
					status = Closed;
			}
			if (sendBytes == -1)
			{
				status = Error;
				std::cerr << "Error sending data: " << strerror(errno) << "\n";
				throw HttpError(InternalServerError, "Internal Server Error");
			}
			return ;
		}
		totalBytesSent += sendBytes;
	}
	this->sendingBuffer.erase(0, totalBytesSent);
}