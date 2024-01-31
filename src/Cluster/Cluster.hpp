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
#include "../Config/ConfigParser.hpp"


class Cluster
{
	private :
		std::map<int, ClientHandler> clientsZone;
		std::vector <Server*> servers;
		int epollFd;
		ClusterConfig config;

	public:
		/**/

	private:
		Server& getServerByClientFd(int fd);
		Server& getServerByFd(int fd);
		void createEpoll();
		void addSocketToEpoll(int fd);
		void eventLoop();
		void handleEvents(struct epoll_event* events, int numEvents);
		bool isServerFd(int fd);
		void handleExistingConnection(int eventFd, uint32_t eventsData);
		void acceptConnections(int serverSocket);
		void cleanup();
	public:
		Cluster(const ClusterConfig &configs);
};


#endif //CLUSTER_HPP