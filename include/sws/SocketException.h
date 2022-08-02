#pragma once

#include <exception>
#include <string>
#include "SocketError.h"

namespace sws
{
	class SocketException : public std::exception
	{
	protected:
		std::string message;

	public:
		const SocketError native_error;

		explicit SocketException(SocketError error);
		SocketException(const char* msg, SocketError error);
		SocketException(std::string msg, SocketError error);

		[[nodiscard]] char const* what() const override;

	private:
		void append_error_string();
	};
}
