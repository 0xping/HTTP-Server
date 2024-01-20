#include "ClientHandler.hpp"


ClientHandler::ClientHandler(int clientFd, int epollFd ,const ServerConfig &serverConfig, const ClusterConfig &clusterConfig)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->serverConfig = serverConfig;
	this->clusterConfig = clusterConfig;
	this->status = Receiving;

	this->counter = 0;
	this->isDir = 0;
	this->isCGIfile = 0;
	this->headersSent = 0;
	this->offset = 0;
	this->lastReceive = 0;
	this->chunkSize = 0;
	this->in = false;
	this->state = startBound;
	this->firstboundary = true;
	// cgi
	this->isCGI = 0;
	this->monitorCGI = 0;
}

ClientHandler::~ClientHandler(){
	std::vector<std::string>::iterator it = tmpFiles.begin();
	for (;it != tmpFiles.end(); it++){
		std::remove(it->c_str());
	}
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
			if (message.headers.find("Host") != message.headers.end()){
				serverConfig = clusterConfig.getServerConfig(serverConfig.ip, serverConfig.port, message.headers["Host"]);;
				RequestParser::serverConfig = serverConfig;
			}
			else
				throw HttpError(BadRequest, "Bad Request check the Host header");
			parseRequest();
			if (!location._return.empty())
				redirect();
			else if (message.method == "GET")
				GetMethod();
			else if (message.method == "DELETE")
				DeleteMethod();
			else if (message.method == "POST")
				PostMethod();
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
		if (status == Sending)
		{
			if (monitorCGI)
			{
				checkCGI();
			}
			else
				SendResponse();
		}
		else if (status == Receiving && lastReceive)
		{
			if (time(0) - lastReceive > 5)
			{
				std::cout << "time out" << std::endl;
				status = Sending;
				throw HttpError(RequestTimeOut,"Request Time Out");
			}
			// SendResponse();
		}
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
			status = Sending;
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
		//write(open("log", O_RDWR | O_CREAT | O_APPEND, 0777), sendingBuffer.toStr().c_str() + totalBytesSent, sendingBuffer.toStr().size() - totalBytesSent);
		ssize_t sendBytes = ::send(this->clientFd, sendingBuffer.toStr().c_str() + totalBytesSent, sendingBuffer.toStr().size() - totalBytesSent, MSG_NOSIGNAL);
		if (sendBytes <= 0)
		{
			status = Closed;
			std::cerr << "Error Sending data: " << strerror(errno) << "\n";
			return ;
		}
		totalBytesSent += sendBytes;
	}
	this->sendingBuffer.erase(0, totalBytesSent);
}