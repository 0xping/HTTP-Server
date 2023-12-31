#include "T_server.hpp"

std::vector<std::string> t_server::getErrorPage(std::string& errn_){
    std::map<std::string, std::string>::iterator it;
    std::map<std::string, std::string> httpStatusCodes;
    std::vector<std::string> re;

    // Add HTTP 1.1 status codes and strings to the map 
    httpStatusCodes["100"] = "Continue";
    httpStatusCodes["101"] = "Switching Protocols";
    httpStatusCodes["200"] = "OK";
    httpStatusCodes["201"] = "Created";
    httpStatusCodes["202"] = "Accepted";
    httpStatusCodes["203"] = "Non-Authoritative Information";
    httpStatusCodes["204"] = "No Content";
    httpStatusCodes["205"] = "Reset Content";
    httpStatusCodes["206"] = "Partial Content";
    httpStatusCodes["300"] = "Multiple Choices";
    httpStatusCodes["301"] = "Moved Permanently";
    httpStatusCodes["302"] = "Found";
    httpStatusCodes["303"] = "See Other";
    httpStatusCodes["304"] = "Not Modified";
    httpStatusCodes["305"] = "Use Proxy";
    httpStatusCodes["307"] = "Temporary Redirect";
    httpStatusCodes["400"] = "Bad Request";
    httpStatusCodes["401"] = "Unauthorized";
    httpStatusCodes["402"] = "Payment Required";
    httpStatusCodes["403"] = "Forbidden";
    httpStatusCodes["404"] = "Not Found";
    httpStatusCodes["405"] = "Method Not Allowed";
    httpStatusCodes["406"] = "Not Acceptable";
    httpStatusCodes["407"] = "Proxy Authentication Required";
    httpStatusCodes["408"] = "Request Timeout";
    httpStatusCodes["409"] = "Conflict";
    httpStatusCodes["410"] = "Gone";
    httpStatusCodes["411"] = "Length Required";
    httpStatusCodes["412"] = "Precondition Failed";
    httpStatusCodes["413"] = "Request Entity Too Large";
    httpStatusCodes["414"] = "Request-URI Too Long";
    httpStatusCodes["415"] = "Unsupported Media Type";
    httpStatusCodes["416"] = "Requested Range Not Satisfiable";
    httpStatusCodes["417"] = "Expectation Failed";
    httpStatusCodes["500"] = "Internal Server Error";
    httpStatusCodes["501"] = "Not Implemented";
    httpStatusCodes["502"] = "Bad Gateway";
    httpStatusCodes["503"] = "Service Unavailable";
    httpStatusCodes["504"] = "Gateway Timeout";
    httpStatusCodes["505"] = "HTTP Version Not Supported";

    it = error_pages.find(errn_);
    if (it != error_pages.end())
        re.push_back(it->second);
    else
        re.push_back("error_pages/" + errn_ + ".html");
    re.push_back(httpStatusCodes[errn_]);
    return re;
}

bool t_server::hasLocation(std::string& location){
    if (locations.find(location) != locations.end())
        return 1;

    std::map<std::string, t_location>::iterator it = locations.begin();

    for (it; it != locations.end(); it++){
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

t_location& t_server::getLocation(std::string& location){
    std::map<std::string, t_location>::iterator it = locations.begin();
    t_location re;
    int max_match = 0;

    for (it; it != locations.end(); it++){
        int i = 0;

        while (location[i] && it->first[i] && location[i] == it->first[i])
            i++;
        if (!it->first[i] && (!location[i] || location[i] == '/') && i > max_match){
            max_match = i;
            re = it->second;
        }
    }
    if (!max_match)
        return locations["/"];
    return re;
}