#pragma once

#include <exception>
#include <string>
#include "SocketError.h"

class SocketException : public std::exception
{
protected:
	std::string message;

public:
	const sws::SocketError native_error;

	explicit SocketException(sws::SocketError error);
	SocketException(const char* msg, sws::SocketError error);
	SocketException(std::string msg, sws::SocketError error);

	char const* what() const override;

private:
	void append_error_string();
};
