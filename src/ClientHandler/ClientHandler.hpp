#pragma once
#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <map>
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
		bool closed;


	private:
		void readFromSocket(int bufferSize = BUFFER_SIZE);
		int loadHeaders(const std::string& data);
	public :
		ClientHandler(int clientFd, int epollFd);
		void refresh();
		void closeConnection();
};

#endif //EVENT_HANDLER_HPP