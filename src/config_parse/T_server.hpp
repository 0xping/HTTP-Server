#pragma once

#include "../../webserv.hpp"
#include "T_location.hpp"

class t_server {
    public:
        int                                 port;    
        std::string                         ip;
        std::string                         server_name;
        size_t                              max_body_size;
        std::string                         body_size;
        std::map<std::string, t_location>   locations;
        std::map<std::string, std::string>  error_pages;

        std::string getErrorPage(std::string& errn_);
        bool hasLocation(std::string& location);
        t_location& getLocation(std::string& location);
};