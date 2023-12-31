#include "webserv.hpp"

std::string httpResponse(std::string& statusCode, std::map<std::string, std::string>& headers, t_server& server){
    std::string re = "http/1.1 " + statusCode + server.getErrorPage(statusCode)[1] + "\r\n";
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++){
        re += it->first + ": " + it->second + "\r\n";
    }
    re += "\r\n";
    return re;
}

