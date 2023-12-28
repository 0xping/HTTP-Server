#pragma once

#include "../../webserv.hpp" 
#include "../utils/utils.hpp"

class t_location {
    public:
        std::string              path;
        std::string              root;
        std::string              _return;
        std::string              upload_path;
        std::vector<std::string> index;
        std::vector<std::string> allow_methods;    
        std::map<std::string, std::string> cgi_path;
        bool                     autoindex;

        bool hasMethod(std::string& method);
        bool hasCGI(std::string& cgi);
        std::string& getCGIPath(std::string& cgi);
};