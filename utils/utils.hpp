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
#include <ctime>
#include <cstdlib>
#include <limits.h>

std::string strtrim(const std::string& str, const std::string& charsToTrim = " \t\n\r");
void trim_spaces(std::string& str, bool both);

std::vector<std::string> splitWithDelimiters(const std::string& str, const std::vector<std::string>& delimiters);
std::vector<std::string> strSplit(const std::string& input, const std::string& delimiters, bool includeEmpty);
std::string* split(std::string str, char sep);

void cerrAndExit(std::string err, int s);
bool isAllSpaces(std::string& str);
void trim_spaces(std::string& str, bool both);
bool isAllDigit(std::string &str);
bool allCharactersAllowed(const std::string& str, const std::string& allowedChars);
std::string toString(int n);
std::string generateUniqueFileName(const std::string& directory = "/tmp", const std::string& extension = "");
std::string getFileExtention(const std::string& file_path);
bool isValidBase(const std::string& number, unsigned int &size, const int base);
std::string postHtmlResponseGenerator(std::vector<std::string>& filesNames);
std::string replaceNewlineWithLiteral(const std::string &input) ;
bool fileExists(const std::string& name);
std::string urlDecode(const std::string& input);

#endif