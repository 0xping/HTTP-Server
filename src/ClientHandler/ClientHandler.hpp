#pragma once
#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <functional>
#include <stdexcept>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctime>
#include <vector>
#include <map>
#include "../RequestParser/RequestParser.hpp"
#include "../../utils/utils.hpp"
#include "../Config/ConfigParser.hpp"
#include "../Config/ServerConfig.hpp"
#include "../Binary/Binary.hpp"


const int BUFFER_SIZE = 1024;

enum ClientStatus {
	Receiving, // ready to Receive
	Sending, // ready to Send
	Error, // an error occurred
	Closed, // connection closed
};



class ClientHandler : public RequestParser {
	private:
		int epollFd;

		std::string postedFileName; // replaced with tmp files
		
		// to remove & close in destruction
		std::vector<std::string> tmpFiles;
		std::vector<int> fds;
	public:
		ClientStatus status;
		Binary readingBuffer;
		Binary sendingBuffer;
		int clientFd;
		ServerConfig serverConfig;
		ClusterConfig clusterConfig;

	private:
		void sendToSocket();
		void readFromSocket(int bufferSize = BUFFER_SIZE);
	public :
		ClientHandler(int clientFd, int epollFd ,const  ServerConfig &serverConfig, const ClusterConfig &config);
		void readyToReceive();
		void readyToSend();


	// send response
	public:		
		std::string file;
		std::string statusCode;
		std::string statusString;
		std::string extraHeaders;
		bool isCGI;		
		
		bool headersSent;

		void setResponseParams(std::string statusCode, std::string statusString, std::string extraHeaders, std::string file, bool isCGI=false);


		void SendResponse();
		std::string generateHeaders();
		std::string getExtension();
		std::string getContentLength();
		std::string getMimeType(std::string ext);

		int offset;

};

#endif //EVENT_HANDLER_HPP