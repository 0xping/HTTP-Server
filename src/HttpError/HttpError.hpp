#pragma once

#ifndef HTTP_ERROR_HPP
#define HTTP_ERROR_HPP

#include <stdexcept>

enum ErrorCode {
	BadRequest = 400,
	NotFound = 404,
	NotImplemented = 501,
	RequestURIToLong = 414,
	MethodNotAllowed = 405,
	InternalServerError = 500,
	// Add more errors
};

class HttpError : public std::runtime_error {
	private:
		ErrorCode errorCode;

	public:
		HttpError(ErrorCode code, const std::string& message);
		ErrorCode getErrorCode() const;

};

#endif // HTTP_ERROR_HPP
