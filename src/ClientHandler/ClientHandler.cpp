#include "ClientHandler.hpp"


ClientHandler::ClientHandler(int clientFd, int epollFd ,const ServerConfig &serverConfig, const ClusterConfig &clusterConfig)
{
	this->epollFd = epollFd;
	this->clientFd = clientFd;
	this->serverConfig = serverConfig;
	this->clusterConfig = clusterConfig;
	this->status = Receiving;
	this->postedFileName = "";
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
			
		}
		// check and call the method DELETE or POST <No GET>
		// to send a request form a method , just append to sendingBuffer
		// status = Sending;
	}
	catch (const HttpError& e)
	{
		status = Error;
		int errorCode = static_cast<int>(e.getErrorCode());
		std::cerr << "HTTP Error (" << errorCode << "): " << e.what() << std::endl;
		// TODO : store the error state in the object and wait for a send event to come
	}
}

void ClientHandler::readyToSend() {
	// Client ready to send data
	try
	{
		if (status == Error)
		{
			// check and send the error page
			// set status to Closed if done sending
			return ;
		}

		if (headersLoaded && status == Sending)
		{

			if (message.method == "GET")
			{
				// call get Method to fill the sendingBuffer and then call sendToSocket
				unsigned char httpResponse[] =  "HTTP/1.1 200 OK \r\n"
								"Content-Type: text/plain\r\n"
								"Content-Length: 12\r\n"
								"\r\n"
								"Hello, World";
				sendingBuffer.append(httpResponse,std::strlen((char *)httpResponse));
				sendToSocket();
			}


			/// set status to Closed if done sending
			status = Closed;
		}
	}
	catch (const HttpError& e)
	{
		status = Error;
		int errorCode = static_cast<int>(e.getErrorCode());
		std::cerr << "HTTP Error (" << errorCode << "): " << e.what() << std::endl;
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



// std::cout << "-------> Headers <------- " << message.headers.size() << "\n\n";
// for (std::map<std::string, std::string>::const_iterator it = message.headers.begin(); it != message.headers.end(); ++it) {
//	 std::cout << it->first << ":" << it->second << '\n';
// }

// std::cout << "\n\n-------> Body  <-------\n" << toRead << '\n';
