
#pragma once

#ifndef BINARY_HPP
#define BINARY_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

class Binary {
	private:
		std::vector<unsigned char> data;
	public:
		Binary();
		Binary(const std::vector<unsigned char>& initialData);
		Binary(const unsigned char* rawData, size_t size);

		void append(const std::vector<unsigned char>& newData);
		void append(const unsigned char* newData, size_t size);

		size_t find(const std::string& pattern) const;

		Binary substr(size_t start, std::string::size_type length = std::string::npos) const;

		size_t size();
		void erase(size_t start, size_t length);
		std::string toStr() const;
		// Overloaded operators
		Binary& operator=(const Binary& other);
		friend std::ostream& operator<<(std::ostream& os, const Binary& binary);
};

#endif // BINARY_HPP


