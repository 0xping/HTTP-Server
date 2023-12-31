#pragma once
#ifndef CLUSTER_HPP
#define CLUSTER_HPP

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
#include "../Server/Server.hpp"

class Cluster
{
	private :
        std::map<int, ClientHandler> clientsZone;
		std::vector <Server*> servers;
		int epollFd;
		void createEpoll();
		void addSocketToEpoll(int fd);
		void eventLoop();
		void handleEvents(struct epoll_event* events, int numEvents);
		bool isServerFd(int fd);
		void handleExistingConnection(int eventFd);
		void acceptConnections(int serverSocket);
		int setNonBlocking(int fd);
		void cleanup();
	public:
		Cluster(std::vector<Config> configs);
		void start();
};


#endif //CLUSTER_HPP