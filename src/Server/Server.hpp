#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

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
#include "../ClientHandler/ClientHandler.hpp"
#include "../Config/Config.hpp"


class Server {
    public:
        Server(const Config& config) ;
        ~Server();
        void start();

    private:
        int serverSocket;
        int epollFd;
        Config serverConfig;
        std::map<int, ClientHandler> clientsZone;
        void handleExistingConnection(int eventFd);
        void initAndBindSocket();
        void listenForConnections();
        void createEpoll();
        void addSocketToEpoll(int fd);
        void eventLoop();
        void acceptConnections();
        int setNonBlocking(int fd);
        void handleEvents(struct epoll_event* events, int numEvents);
        struct addrinfo* setupAddressInfo();
        void bindToAddress(struct addrinfo* result);
        void cleanup();

};



#endif