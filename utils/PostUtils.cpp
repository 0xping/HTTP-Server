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
    std::string htmlResp = ""
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>File Upload Success</title>"
        "<style>"
            "body {"
                "background-color: #212121;"
                "color: #ffffff;"
                "font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
                "text-align: center;"
                "padding: 50px;"
                "margin: 0;"
            "}"
            ".success-message {"
                "background-color: #43a047;"
                "color: #ffffff;"
                "padding: 20px;"
                "border-radius: 5px;"
                "box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);"
            "}"
        "</style>"
    "</head>"
    "<body>"
        "<div class=\"success-message\">"
            "<h1>File Uploaded Successfully!</h1>"
            "<p>The following files has been successfully uploaded to the server.</p>"
            "<ul>";
                for (size_t i = 0; i < filesNames.size() - 1; i++)
                    htmlResp += "<li> <a href=" + (std::string)(filesNames[i].c_str() + 6) + ">"  + filesNames[i] + "</a></li>" ;
            htmlResp += "</ul>"
        "</div>"
    "</body>";
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