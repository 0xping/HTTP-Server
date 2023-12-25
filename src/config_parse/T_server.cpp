#include "T_server.hpp"

std::string t_server::getErrorPage(std::string& errn_){
    std::map<std::string, std::string>::iterator it;

    it = error_pages.find(errn_);
    if (it != error_pages.end())
        return it->second;
    return ("error_pages/" + errn_ + ".html");
}

bool t_server::hasLocation(std::string& location){
    if (locations.find(location) != locations.end())
        return 1;
    return 0;
}

t_location& t_server::getLocation(std::string& location){
    return locations[location];
}