#pragma once
#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
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
		std::string toSend;
	public:
		bool headersLoaded;
		Binary toRead;
		int clientFd;
		bool closed;
		ServerConfig serverConfig;
		ClusterConfig clusterConfig;


	private:
		void readFromSocket(int bufferSize = BUFFER_SIZE);
		int loadHeaders();
	public :
		ClientHandler(int clientFd, int epollFd ,const  ServerConfig &serverConfig, const ClusterConfig &config);
		void closeConnection();
		void receive();
		void send();

};

#endif //EVENT_HANDLER_HPP