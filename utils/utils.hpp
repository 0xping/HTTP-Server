#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

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
#include <vector>
#include <string>
#include <iostream>

std::string strtrim(const std::string& str, const std::string& charsToTrim = " \t\n\r");
void trim_spaces(std::string& str, bool both);

std::vector<std::string> splitWithDelimiters(const std::string& str, const std::vector<std::string>& delimiters);
std::vector<std::string> strSplit(const std::string& input, const std::string& delimiters);
std::string* split(std::string str, char sep);

void cerrAndExit(std::string err, int s);
bool isAllSpaces(std::string& str);
void trim_spaces(std::string& str, bool both);
bool isAllDigit(std::string &str);
bool allCharactersAllowed(const std::string& str, const std::string& allowedChars);

std::string getFileExtention(const std::string& file_path);
#endif