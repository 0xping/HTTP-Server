#pragma once
#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <map>
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
		bool headersLoaded;
		std::string toSend;
	public:
		std::string toRead;
		int clientFd;
		ServerConfig serverConfig;
		ClusterConfig clusterConfig;

	// send response
	private:	
		bool headersSent;		
		int Offset;

		void SendResponse(std::string statusCode, std::map<std::string, std::string> headers, std::string file, bool isCgi=false);
		std::string getMimeType(std::string ext);
		std::string getExtension(std::string& filename);
		std::string getContentLength(std::ifstream& file);
		std::string generateHeaders(std::string& statusCode, std::map<std::string, std::string>& headers, std::string& filename, std::ifstream& file, bool isCgi=false);

	public:
		bool closed;		

	// parsing
	private:
		Location location;
		std::string full_location;
		std::string query;
		std::string FileName;
		std::string cgi_path;
		bool isCgipath;

		void proccessLocation();
		bool isCgiFile(std::string& filename);
		void execCGI(std::string& filename);

	private:
		void readFromSocket(int bufferSize = BUFFER_SIZE);
		int loadHeaders(const std::string& data);
		
		//parsing
		void checkPath();
		bool isDir;

	public:
		ClientHandler(int clientFd, int epollFd ,const  ServerConfig &serverConfig, const ClusterConfig &config);
		void refresh();
		void closeConnection();

};

#endif //EVENT_HANDLER_HPP