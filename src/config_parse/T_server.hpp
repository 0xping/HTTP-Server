#pragma once

#include "../../webserv.hpp"
#include "T_location.hpp"
#include "../utils/utils.hpp"

class t_server {        
    std::map<std::string, t_location>   locations;
    std::map<std::string, std::string>  error_pages;
    public:
        int                                 port;    
        std::string                         ip;
        std::string                         server_name;
        size_t                              max_body_size;
        std::string                         body_size;


        std::vector<std::string> getErrorPage(std::string& errn_);
        bool hasLocation(std::string& location);
        t_location& getLocation(std::string& location);
};