#include "utils.hpp"

std::string urlDecode(const std::string& input) {
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            int hexValue;
            if (sscanf(input.substr(i + 1, 2).c_str(), "%x", &hexValue) == 1) {
                result += static_cast<char>(hexValue);
                i += 2;
            } else {
                result += input[i];
            }
        } else if (input[i] == '+') {
            result += ' ';
        } else {
            result += input[i];
        }
    }

    return result;
}
