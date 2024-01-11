#include "utils.hpp"

bool isValidHex(const std::string& hex, unsigned int &size)
{
    for (size_t i = 0; i < hex.length(); i++)
    {
        if (!std::isxdigit(hex[i]))
            return false;
    }

    char *endPtr;
    size = std::strtoul(hex.c_str(), &endPtr, 16);
    if (!*endPtr || size == UINT_MAX)
        return false;
    return true;
}

// bool isValidInt(const std::string& contentLength, unsigned int counter)
// {

// }