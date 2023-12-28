#pragma once

#include "../../webserv.hpp"

/* ----- utils ----- */
void cerrAndExit(std::string err, int s);
bool isAllSpaces(std::string& str);
void trim_spaces(std::string& str, bool both);
bool isAlldigit(std::string &str);
std::string* split(std::string str, char sep);