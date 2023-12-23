#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <map>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <cctype>
#include <unistd.h>

#include "config_parse/Config.hpp"

class Config;

/* ----- utils ----- */
void cerrAndExit(std::string err, int s);
bool isAllSpaces(std::string& str);
void trim_spaces(std::string& str, bool both);
bool isAlldigit(std::string &str);
std::string* split(std::string str, char sep);