#include "T_location.hpp"

bool t_location::hasMethod(std::string& method){
    if (allow_methods.empty() && method == "GET")
        return 1;
    std::vector<std::string>::iterator it;

    it = std::find(allow_methods.begin(), allow_methods.end(), method);
    if (it != allow_methods.end())
        return 1;
    return 0;
}

bool t_location::hasCGI(std::string& cgi){
    if (cgi_path.find(cgi) != cgi_path.end())
        return 1;
    return 0;
}

std::string& t_location::getCGIPath(std::string& cgi){
    return cgi_path[cgi];
}