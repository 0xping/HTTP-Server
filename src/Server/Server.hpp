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
#include "../config_parse/Config.hpp"


// Server.hpp



class Server {
public:
    Server(const Config& config) ;
    ~Server();

    void start();

private:
    int serverSocket;
    int epollFd;
    Config serverConfig;

    void initAndBindSocket();
    void listenForConnections();
    void createEpoll();
    void addSocketToEpoll(int fd);
    void eventLoop();
    void acceptConnections();
    void handleConnection(int clientSocket);
    void cleanup();
    int setNonBlocking(int fd);
};



#endif