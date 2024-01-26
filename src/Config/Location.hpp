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
#include "../../utils/utils.hpp"

class Location {
    public:
        std::string              path;
        std::string              root;
        std::string              _return;
        std::string              upload_path;
        std::vector<std::string> index;
        std::vector<std::string> allow_methods;
        std::map<std::string, std::string> cgi_path;
        bool                     autoindex;
        Location(){autoindex = false;};
        bool hasMethod(std::string& method);
        bool hasCGI(std::string& cgi);
};