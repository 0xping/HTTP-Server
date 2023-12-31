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
#include "../Config/ConfigParser.hpp"
#include <set>

class Server {
    public:
        Server(const ServerConfig& config, const ClusterConfig& clusterConfig);
        ~Server();
        void cleanup();
        int serverSocket;
        ServerConfig serverConfig;
        std::set<int> connectedClients;

    private:
        ConfigParser clusterConfig;
        void listenForConnections();
        struct addrinfo* setupAddressInfo();
        void bindToAddress(struct addrinfo* result);
};



#endif