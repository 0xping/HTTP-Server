#include "utils.hpp"

bool isValidBase(const std::string& number, unsigned int &size, const int base)
{
    char *endPtr;
    size = std::strtoul(number.c_str(), &endPtr, base);
    if (*endPtr || size == UINT_MAX)
        return false;
    return true;
}

std::string postHtmlResponseGenerator(std::vector<std::string>& filesNames)
{
    std::string htmlResp = "\r\n";
    htmlResp += "<!DOCTYPE html>";
    htmlResp += "<html>";
    htmlResp += "<head>";
    htmlResp += "<title>File Upload Success</title>";
    htmlResp += "</head>";
    htmlResp += "<body>";
    if (filesNames.size() > 1)
    {
        htmlResp += "<h1>Files Uploaded Successfully!</h1>";
        htmlResp += "<p>The following files have been uploaded:</p>";
    }
    else
    {
        htmlResp += "<h1>File Uploaded Successfully!</h1>";
        htmlResp += "<p>The following file have been uploaded:</p>";
    }
    htmlResp += "<ul>";
    for (size_t i = 0; i < filesNames.size(); i++)
        htmlResp += "<ul>" + filesNames[i] + "</ul>";
    
    htmlResp += "</ul>";
    htmlResp += "</body>";
    htmlResp += "</html>";
    return htmlResp;
}

std::string replaceNewlineWithLiteral(const std::string &input) 
{
    std::string result = input;
    size_t pos = 0;

    while ((pos = result.find("\r\n", pos)) != std::string::npos) 
    {
        result.replace(pos, 2, "\\r\\n");
        pos += 4; // Advance position (skip replaced string)
    }

    return result;
}