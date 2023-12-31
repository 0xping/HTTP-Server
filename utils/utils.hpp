#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <iostream>

std::string strtrim(const std::string& str, const std::string& charsToTrim = " \t\n\r");
std::vector<std::string> splitWithDelimiters(const std::string& str, const std::vector<std::string>& delimiters);

#endif