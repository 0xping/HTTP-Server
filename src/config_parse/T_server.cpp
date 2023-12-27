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

    std::map<std::string, t_location>::iterator it = locations.begin();

    for (it; it != locations.end(); it++){
        int i = 0;

        while (location[i] && it->first[i] && location[i] == it->first[i])
            i++;
        if (!location[i] || !it->first[i])
            return 1;
    }
    return 0;
}

t_location& t_server::getLocation(std::string& location){
    std::map<std::string, t_location>::iterator it = locations.begin();
    t_location& re;
    int max_match = 0;

    for (it; it != locations.end(); it++){
        int i = 0;

        while (location[i] && it->first[i] && location[i] == it->first[i])
            i++;
        if ((!location[i] || !it->first[i]) && i > max_match){
            max_match = i;
            re = it->second;
        }
    }
    return re;
}