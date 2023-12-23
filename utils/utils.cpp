#include "../webserv.hpp"

void cerrAndExit(std::string err, int s){
    std::cerr << err << std::endl;
    std::exit(s);
}

bool isAllSpaces(std::string& str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isspace(*it)) {
            return false;
        }
    }
    return true;
}

void trim_spaces(std::string& str, bool both){
    /* if both is true, both trailing and leading are removed
        otherwise, only trailing spaces are removed.*/
    if (both){
        int i = 0;
        while (str[i] && std::isspace(str[i]))
            i++;
        str = &str[i];
    }
    int i = str.length() - 1;
    while (std::isspace(str[i]) && i)
        i--;
    str.resize(i + 1);
}

bool isAlldigit(std::string &str){
    for (std::string::iterator it = str.begin(); it != str.end(); it++){
        if (!std::isdigit(*it))
            return 0;
    }
    return 1;
}

std::string* split(std::string str, char sep){
    std::string *re = new std::string[2];

    size_t i = 0;
    while (str[i] && str[i] != sep) 
        i++;
    if (i != str.length())
        re[1] = &str[i + 1];
    re[0] = str;
    re[0].resize(i); 
    return re;
}