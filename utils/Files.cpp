#include "utils.hpp"
#include <string>

std::string getFileExtention(const std::string& filePath) {
	size_t pos = filePath.find_last_of(".");
	if (pos == std::string::npos)  // No extension.
		return "";
	return filePath.substr(pos + 1);  // Return everything after the period.
}
