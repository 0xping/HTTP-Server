#include "ServerConfig.hpp"

std::string ServerConfig::getErrorPage(std::string& errn_){
    std::map<std::string, std::string>::iterator it;
    std::string re;

    it = error_pages.find(errn_);
    if (it != error_pages.end())
        re = it->second;
    else
        re = "error_pages/" + errn_ + ".html";
    return re;
}

bool ServerConfig::hasLocation(std::string& location){
    if (locations.find(location) != locations.end())
        return 1;

    for (std::map<std::string, Location>::iterator it = locations.begin(); it != locations.end(); it++){
        int i = 0;

        while (location[i] && it->first[i] && location[i] == it->first[i])
            i++;
        if (!it->first[i] && (!location[i] || location[i] == '/'))
            return 1;
    }
    if (locations.find("/") != locations.end())
        return 1;

    return 0;
}

Location& ServerConfig::getLocation(std::string& location){
    Location *re = NULL;
    int max_match = 0;

    for (std::map<std::string, Location>::iterator it = locations.begin(); it != locations.end(); it++){
        int i = 0;
        while (location[i] && it->first[i] && location[i] == it->first[i])
            i++;
        if (!it->first[i] && (!location[i] || location[i] == '/') && i > max_match){
            max_match = i;
            re = &it->second;
        }
    }    
    if (!max_match)
        return locations["/"];

    return *re;
}