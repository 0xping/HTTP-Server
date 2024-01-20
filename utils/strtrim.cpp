#include <iostream>
#include <cctype>

std::string strtrim(const std::string& str, const std::string& charsToTrim = " \t\n\r") {
    size_t first = str.find_first_not_of(charsToTrim);
    if (first == std::string::npos) {
        // String is all whitespace
        return "";
    }

    size_t last = str.find_last_not_of(charsToTrim);
    return str.substr(first, last - first + 1);
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