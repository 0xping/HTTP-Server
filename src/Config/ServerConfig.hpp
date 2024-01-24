#pragma once

// #include "../../webserv.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <map>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <cctype>
#include <unistd.h>
#include "Location.hpp"
#include "../../utils/utils.hpp"
class ServerConfig {

    public:
        int                                 port;
        std::string                         ip;
        std::vector<std::string>            server_names;
        size_t                              max_body_size;
        std::string                         body_size;
        std::map<std::string, Location>   locations;
        std::map<std::string, std::string>  error_pages;
        std::string getErrorPage(std::string& errn_);
        bool hasLocation(std::string& location);
        Location& getLocation(std::string& location);
};