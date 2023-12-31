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
