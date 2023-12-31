#include "utils.hpp"

bool isAllSpaces(std::string& str) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isspace(*it)) {
            return false;
        }
    }
    return true;
}

bool isAllDigit(std::string &str){
    for (std::string::iterator it = str.begin(); it != str.end(); it++){
        if (!std::isdigit(*it))
            return 0;
    }
    return 1;
}

