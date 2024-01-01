#include "Binary.hpp"

Binary::Binary() {}
Binary::Binary(const std::vector<unsigned char>& initialData) : data(initialData) {}
Binary::Binary(const unsigned char* rawData, size_t size) : data(rawData, rawData + size) {}

void Binary::append(const std::vector<unsigned char>& newData) {
	data.insert(data.end(), newData.begin(), newData.end());
}

void Binary::append(const unsigned char* newData, size_t size) {
	data.insert(data.end(), newData, newData + size);
}

size_t Binary::find(const std::string& pattern) const {
	if (pattern.empty()) {
		return std::string::npos;  // Empty pattern cannot be found
	}

	std::vector<unsigned char>::const_iterator result = std::search(data.begin(), data.end(), pattern.begin(), pattern.end());

	if (result != data.end()) {
		return std::distance(data.begin(), result);
	} else {
		return std::string::npos;  // Pattern not found
	}
}

Binary Binary::substr(size_t start, std::string::size_type length) const {
	length = std::min(length, data.size() - start);
	return Binary(&data[start], length);
}

std::ostream& operator<<(std::ostream& os, const Binary& binary) {
	for (std::vector<unsigned char>::const_iterator it = binary.data.begin(); it != binary.data.end(); ++it) {
		os << std::hex << static_cast<int>(*it) << " ";
	}
	os << std::dec;
	return os;
}

Binary& Binary::operator=(const Binary& other) {
	if (this != &other) {
		this->data = other.data;
	}
	return *this;
}

std::string Binary::toStr() const {
	std::string result(data.begin(), data.end());
	return result;
}

void Binary::erase(size_t start, size_t length) {
    if (start < data.size()) {
        data.erase(data.begin() + start, data.begin() + std::min(start + length, data.size()));
    }
}

size_t Binary::size() {return data.size();}