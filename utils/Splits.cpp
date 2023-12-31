#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#include <vector>
#include <string>
#include <algorithm>

std::vector<std::string> splitWithDelimiters(const std::string& str, const std::vector<std::string>& delimiters) {
	std::vector<std::string> result;
	size_t start = 0;

	for (size_t i = 0; i < str.size(); ++i) {
		for (std::vector<std::string>::const_iterator it = delimiters.begin(); it != delimiters.end(); ++it) {
			const std::string& delimiter = *it;
			if (str.substr(i, delimiter.size()) == delimiter) {
				result.push_back(str.substr(start, i - start));
				start = i + delimiter.size();
				i = start - 1; // Adjust for the next iteration
				break; // Move to the next character in the str
			}
		}
	}

	result.push_back(str.substr(start)); // Add the remaining part

	return result;
}

std::vector<std::string> strSplit(const std::string& input, const std::string& delimiters) {
    std::vector<std::string> result;
    size_t start = 0, end = 0;

    while ((end = input.find_first_of(delimiters, start)) != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + 1;
    }

    result.push_back(input.substr(start));

    return result;
}

std::string* split(std::string str, char sep){
    std::string *re = new std::string[2];

    size_t i = 0;
    while (str[i] && str[i] != sep)
        i++;
    if (i != str.length())
        re[1] = &str[i + 1];
    re[0] = str;
    re[0].resize(i);
    return re;
}