#include "utils.hpp"

bool isValidBase(const std::string& number, unsigned int &size, const int base)
{
    char *endPtr;
    size = std::strtoul(number.c_str(), &endPtr, base);
    if (!*endPtr || size == UINT_MAX)
        return false;
    return true;
}


// bool isValidInt(const std::string& contentLength, unsigned int counter)
// {

// }