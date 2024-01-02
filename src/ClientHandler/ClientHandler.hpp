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
#include "../../utils/utils.hpp"
#include "../Config/ConfigParser.hpp"
#include "../Config/ServerConfig.hpp"
#include "../Binary/Binary.hpp"

struct requestMessage
{
	std::string method;
	std::string location;
	std::map<std::string, std::string> headers;
	// std::string body;
};

const int BUFFER_SIZE = 1024;

class ClientHandler
{
	private:
		int epollFd;
		requestMessage message;
		//int total;
		

	public:
		bool headersLoaded;
		Binary toRead;
		std::string FileName;
		int clientFd;
		ServerConfig serverConfig;
		ClusterConfig clusterConfig;

	// send response
	private:	
		bool headersSent;
		bool isCgiSending;		

		void SendResponse(std::string statusCode, std::map<std::string, std::string> headers, std::string file, bool isCgi=false);
		std::string getMimeType(std::string ext);
		std::string getExtension();
		std::string getContentLength();
		std::string generateHeaders(std::string& statusCode, std::map<std::string, std::string>& headers, bool isCgi=false);

	public:
		bool closed;		

	// parsing
	private:
		Location location;
		std::string full_location;
		std::string query;
		int toSendFd;
		std::string cgi_path;
		bool isCgipath;

		void proccessLocation();
		bool isCgiFile();
		void execCGI(std::string& filename);

	private:
		void readFromSocket(int bufferSize = BUFFER_SIZE);
		int loadHeaders();
		
		//parsing
		void checkPath();
		bool isDir;


	public :
		ClientHandler(int clientFd, int epollFd ,const  ServerConfig &serverConfig, const ClusterConfig &config);
		void closeConnection();
		void receive();
		void send();

};

#endif //EVENT_HANDLER_HPP