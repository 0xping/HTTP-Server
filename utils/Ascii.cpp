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

bool allCharactersAllowed(const std::string& str, const std::string& allowedChars) {
    for (std::size_t i = 0; i < str.length(); ++i) {
        if (allowedChars.find_first_of(str[i]) == std::string::npos) {
            return false; // Found a character not in allowedChars
        }
    }
    return true; // All characters are in allowedChars
}

// convert an int to a string
std::string toString(int n){
    std::stringstream ss;
    ss << n;
    return ss.str();
}