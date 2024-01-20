#include "HttpError.hpp"

HttpError::HttpError(ErrorCode code, const std::string& message)
	: std::runtime_error(message), errorCode(code) {}

ErrorCode HttpError::getErrorCode() const {
	return errorCode;
}
